#include <memory>
#include <string>
#include "GlobalSettings.h"
#include "HttpWebSocket.h"
#include "HttpClientBase.h"

bool HttpWebSocket::ProcessIncomingMessage(std::unique_ptr<HttpClientBase> &client, std::string &outTextReceived)
{
    int availableBytes = client->BytesAvailable();
    if (availableBytes == 0) return false;

    //-------------------------------------------------
    // If the client is abusing with packets, kill him!
    if (availableBytes >= MAX_HTTP_REQUEST_SIZE)
    {
        client->SetState(CLOSING_CONNECTION);
        return false;
    }
    //-------------------------------------------------

    // First, let's get all available data
    auto buffer = std::vector<char>(availableBytes);
    int numBytesRead;
    // If we didn't receive data, leave
    if (!client->ReadData(availableBytes, buffer, &numBytesRead))
        return false;

    // Creating a frame from the bytes received
    std::unique_ptr<WebSocketFrame> sf(new WebSocketFrame());
    for (int counter = 0; counter < numBytesRead; counter++)
    {
        bool more = sf->HasNextInt((unsigned char)buffer[counter]);
        bool lastByte = counter == numBytesRead - 1;
        // Last byte shall be last expected char in the packet
        if ((more && lastByte) || (!more && !lastByte))
            return false;
    }

    // Processing any PING request
    if (sf->GetFrameType() == PING)
    {
        std::unique_ptr<WebSocketFrame> frame(WebSocketFrame::CreatePongFrame());
        auto bytesEnvio = frame->ToBytes();
        client->WriteData(bytesEnvio);
    }

    // Processing any CLOSE request
    else if (sf->GetFrameType() == CONNECTION_CLOSE)
    {
        std::unique_ptr<WebSocketFrame> frame(WebSocketFrame::CreateCloseFrame());
        auto bytesEnvio = frame->ToBytes();
        client->WriteData(bytesEnvio);
        client->SetState(CLOSING_CONNECTION);
    }

    // Text received
    else if (sf->GetFrameType() == TEXT_DATA)
    {
        outTextReceived = sf->GetTextData();
        return true;
    }

    return false;
}

void HttpWebSocket::SendTextMessage(std::unique_ptr<HttpClientBase> &client, std::string textOut)
{
    std::unique_ptr<WebSocketFrame> frame(WebSocketFrame::CreateTextFrame(textOut));
    auto bytesEnvio = frame->ToBytes();
    client->WriteData(bytesEnvio);
}

void HttpWebSocket::SendPingRequest(std::unique_ptr<HttpClientBase> &client)
{
    std::unique_ptr<WebSocketFrame> frame(WebSocketFrame::CreatePingFrame());
    auto bytesEnvio = frame->ToBytes();
    client->WriteData(bytesEnvio);
}

void HttpWebSocket::SendCloseRequest(std::unique_ptr<HttpClientBase> &client)
{
    std::unique_ptr<WebSocketFrame> frame(WebSocketFrame::CreateCloseFrame());
    auto bytesEnvio = frame->ToBytes();
    client->WriteData(bytesEnvio);
}

WebSocketFrame::WebSocketFrame() : fin(false), rsv1(false), rsv2(false), rsv3(false), opcode(0), mask(false), payloadLength(0), extendedPayloadLength(0), maskKey(0), bytesRead(0), nextGroupToRead(0) { }
WebSocketFrame::WebSocketFrame(bool fin, FrameType fType) : fin(fin), rsv1(false), rsv2(false), rsv3(false), opcode(fType), mask(false), payloadLength(0), extendedPayloadLength(0), maskKey(0), bytesRead(0), nextGroupToRead(0) { }
int WebSocketFrame::GetFrameType() { return opcode; }
std::vector<char> WebSocketFrame::GetBinaryData() { return payloadData; }
std::string WebSocketFrame::GetTextData() { return std::string(payloadData.begin(), payloadData.end()); }

bool WebSocketFrame::HasNextInt(unsigned char frameByte)
{
    bytesRead++;

    if (bytesRead == 1)
    {
        fin = ((frameByte >> 7) & 1) == 1;
        rsv1 = ((frameByte >> 6) & 1) == 1;
        rsv2 = ((frameByte >> 5) & 1) == 1;
        rsv3 = ((frameByte >> 4) & 1) == 1;
        opcode = (char)(frameByte & 0xf);
        return true;
    }
    else if (bytesRead == 2)
    {
        mask = ((frameByte >> 7) & 1) == 1;
        payloadLength = (char)(frameByte & 127);

        nextGroupToRead = payloadLength > 25 ? 3 : (mask ? 11 : 15);

        // Se tiver o bloco de mask para ler ou se tivermos dados para ler, retornar verdadeiro
        return (mask || payloadLength > 0);
    }
    else if (nextGroupToRead >= 3 && nextGroupToRead <= 10)
    {
        extendedPayloadLength |= ((unsigned long)frameByte << (payloadLength == 126 ? 4 - nextGroupToRead : 10 - nextGroupToRead) * 8);
        nextGroupToRead++;

        // Se chegarmos ao final do Extended Payload e não tivermos mask pra ler, pulamos para o grupo do Payload Data
        if (nextGroupToRead == 11 && !mask) nextGroupToRead = 15;

        // Se estamos no grupo do Extended Payload, é porque tem mais dados pra ler, retornar TRUE
        return true;
    }
    else if (nextGroupToRead >= 11 && nextGroupToRead <= 14)
    {
        maskKey |= (frameByte << (14 - nextGroupToRead) * 8);
        nextGroupToRead++;

        // Se tivermos mais máscara pra ler ou se tivermos dados pra ler, retornaremos TRUE para ler o Payload Data
        return (nextGroupToRead <= 14 || payloadLength > 0);
    }
    else
    {
        char maskByte = (char)((maskKey >> 8 * (3 - (payloadData.size() % 4))) & 0xff);
        payloadData.push_back((char)(frameByte ^ maskByte));
        unsigned long expectedSize = (payloadLength < 125 ? payloadLength : extendedPayloadLength);
        return (unsigned long)payloadData.size() < expectedSize;
    }
}

WebSocketFrame *WebSocketFrame::CreateCloseFrame()
{
    WebSocketFrame *sf = new WebSocketFrame(true, CONNECTION_CLOSE);
    return sf;
}

WebSocketFrame *WebSocketFrame::CreatePingFrame()
{
    WebSocketFrame *sf = new WebSocketFrame(true, PING);
    return sf;
}

WebSocketFrame *WebSocketFrame::CreatePongFrame()
{
    WebSocketFrame *sf = new WebSocketFrame(true, PONG);
    return sf;
}

WebSocketFrame *WebSocketFrame::CreateTextFrame(std::string Data)
{
    std::vector<char> chars(Data.begin(), Data.end());
    WebSocketFrame *sf = new WebSocketFrame(true, TEXT_DATA);
    sf->payloadData = chars;

    if (Data.length() <= 125) sf->payloadLength = (char)Data.length();
    else if (Data.length() < 65535) { sf->payloadLength = 126; sf->extendedPayloadLength = (unsigned long)Data.length(); }
    else { sf->payloadLength = 127; sf->extendedPayloadLength = (unsigned long)Data.length(); }

    return sf;
}

std::vector<char> WebSocketFrame::ToBytes()
{
    std::vector<char> bytes;
    char b1 = (char)(((fin ? 1 : 0) << 7) | ((rsv1 ? 1 : 0) << 6) | ((rsv2 ? 1 : 0) << 5) |
                ((rsv3 ? 1 : 0) << 4) | (opcode & 0xf));
    bytes.push_back(b1);

    char pLengthField = (payloadData.size() <= 125 ? (char)payloadData.size() : (payloadData.size() < 65535 ? (char)126 : (char)127));
    bytes.push_back((char)((mask ? 128 : 0) | (pLengthField & 127)));

    if (payloadLength == 126)
    {
        bytes.push_back((char)((extendedPayloadLength >> 8) & 0xff));
        bytes.push_back((char)(extendedPayloadLength & 0xff));
    }

    if (payloadLength == 127)
    {
        bytes.push_back((char)((extendedPayloadLength >> 56) & 0xff));
        bytes.push_back((char)((extendedPayloadLength >> 48) & 0xff));
        bytes.push_back((char)((extendedPayloadLength >> 40) & 0xff));
        bytes.push_back((char)((extendedPayloadLength >> 32) & 0xff));
        bytes.push_back((char)((extendedPayloadLength >> 24) & 0xff));
        bytes.push_back((char)((extendedPayloadLength >> 16) & 0xff));
        bytes.push_back((char)((extendedPayloadLength >> 8) & 0xff));
        bytes.push_back((char)((extendedPayloadLength >> 0) & 0xff));
    }

    if (mask)
    {
        bytes.push_back((char)((maskKey >> 24) & 0xff));
        bytes.push_back((char)((maskKey >> 16) & 0xff));
        bytes.push_back((char)((maskKey >> 8) & 0xff));
        bytes.push_back((char)((maskKey >> 0) & 0xff));
    }

    bytes.insert(bytes.end(), payloadData.begin(), payloadData.end());

    return bytes;
}
