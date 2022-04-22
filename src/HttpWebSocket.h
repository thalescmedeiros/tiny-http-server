#ifndef H_HttpWebSocket
#define H_HttpWebSocket

#include <iostream>
#include <memory>
#include <vector>
#include <string>

class HttpClientBase;

enum FrameType
{
    CONTINUATION = 0x00,
    TEXT_DATA = 0x01,
    BINARY_DATA = 0x02,
    NON_CONTROL1 = 0x03,
    NON_CONTROL2 = 0x04,
    NON_CONTROL3 = 0x05,
    NON_CONTROL4 = 0x06,
    NON_CONTROL5 = 0x07,
    CONNECTION_CLOSE = 0x08,
    PING = 0x09,
    PONG = 0x0A
};

class WebSocketFrame
{
    public:
        WebSocketFrame();
        WebSocketFrame(bool fin, FrameType fType);

        int GetFrameType();
        std::vector<char> GetBinaryData();
        std::string GetTextData();
        bool HasNextInt(unsigned char frameByte);
        std::vector<char> ToBytes();

        static WebSocketFrame *CreateCloseFrame();
        static WebSocketFrame *CreatePingFrame();
        static WebSocketFrame *CreatePongFrame();
        static WebSocketFrame *CreateTextFrame(std::string Data);

    private:
        bool fin;
        bool rsv1;
        bool rsv2;
        bool rsv3;
        char opcode;
        bool mask;
        char payloadLength;
        unsigned long long extendedPayloadLength;
        int maskKey;
        std::vector<char> payloadData;

        int bytesRead;
        int nextGroupToRead;

        //------------------------Frame Description-----------------------|
        //----------------------------------------------------------------|
        //|0                   1                   2                   3  |
        //|0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1|
        //+-+-+-+-+-------+-+-------------+-------------------------------+
        //|F|R|R|R| opcode|M| Payload len |    Extended payload length    |
        //|I|S|S|S|  (4)  |A|     (7)     |             (16/64)           |
        //|N|V|V|V|       |S|             |   (if payload len==126/127)   |
        //| |1|2|3|       |K|             |                               |
        //+-+-+-+-+-------+-+-------------+ - - - - - - - - - - - - - - - +
        //|     Extended payload length continued, if payload len == 127  |
        //+ - - - - - - - - - - - - - - - +-------------------------------+
        //|                               |Masking-key, if MASK set to 1  |
        //+-------------------------------+-------------------------------+
        //|  Masking-key(continued)       |          Payload Data         |
        //+-------------------------------- - - - - - - - - - - - - - - - +
        //:                      Payload Data continued...                :
        //+ - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - +
        //|                      Payload Data continued...                |
        //+---------------------------------------------------------------+
};

class HttpWebSocket
{
    public:
        static bool ProcessIncomingMessage(std::unique_ptr<HttpClientBase> &client, std::string &outTextReceived);
        static void SendTextMessage(std::unique_ptr<HttpClientBase> &client, std::string textOut);
        static void SendPingRequest(std::unique_ptr<HttpClientBase> &client);
        static void SendCloseRequest(std::unique_ptr<HttpClientBase> &client);
};

#endif
