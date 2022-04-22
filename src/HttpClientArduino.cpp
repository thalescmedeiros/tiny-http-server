#ifdef ARDUINO

#include <memory>
#include <vector>
#include <string>
#include "HttpServerControl.h"
#include "HttpClientBase.h"
#include "HttpClientArduino.h"

HttpClient::HttpClient(std::unique_ptr<HttpServerControl> &server, WiFiClient client, std::string ipv4, int clientPort)
    : HttpClientBase(server, ipv4, clientPort), client(client)
{
}

int HttpClient::GetClient() { return client; }

bool HttpClient::ReadData(int NumBytes, std::vector<char> &Buffer, int *BytesRead)
{
    int bytesToRead = HTTP_READ_BUFFER_SIZE > NumBytes ? NumBytes : HTTP_READ_BUFFER_SIZE;
    *BytesRead = client.readBytes(Buffer, bytesToRead);
    return true;
}

void HttpClient::ClearReadBuffer()
{
    // Defining how many bytes we'll read (based on max buffer)
    int availableBytes = this->BytesAvailable();
    int bufferSize = HTTP_READ_BUFFER_SIZE > availableBytes ? availableBytes : HTTP_READ_BUFFER_SIZE;

    std::vector<char> buffer(bufferSize);
    int totalBytesRead = 0;

    // Reading everything
    while (totalBytesRead < availableBytes)
    {
        int bytesRead = client.readBytes(buffer, bufferSize);
        if (bytesRead <= 0) break;
        else totalBytesRead += bytesRead;
    }
}

int HttpClient::BytesAvailable() { return client.available(); }

bool HttpClient::IsConnected() { return client.connected(); }

void HttpClient::Die() { client.stop(); }

#endif
