#ifdef __WIN64

#define DEBUG
//#define ERRORS_ANALYSIS

#include <memory>
#include <vector>
#include <WinSock2.h>
#include "HttpServerControl.h"
#include "HttpClientWinsock.h"

HttpClient::HttpClient(std::unique_ptr<HttpServerControl> &server, SOCKET socket_client, std::string ipv4, int clientPort)
    : HttpClientBase(server, ipv4, clientPort)
{
    this->socket_client = socket_client;
    this->isConnected = true;
}

bool HttpClient::IsConnected() { return isConnected; }

bool HttpClient::ReadData(int NumBytes, std::vector<char> &Buffer, int *BytesRead)
{
    // Defining how many bytes we'll read (based on max buffer)
    int bytesToRead = HTTP_READ_BUFFER_SIZE > NumBytes ? NumBytes : HTTP_READ_BUFFER_SIZE;
    if (bytesToRead != NumBytes) Buffer.resize(bytesToRead);

    // Reading everything
    *BytesRead = recv(socket_client, &Buffer[0], bytesToRead, 0);   // /*MSG_PEEK |*/ MSG_DONTWAIT

#ifdef DEBUG
std::cout << "(" << socket_client << "): " << *BytesRead << " bytes received\n";
#endif

    if (*BytesRead == 0) isConnected = false;
    else if (*BytesRead == SOCKET_ERROR)
    {
        Buffer.clear();
        *BytesRead = 0;
        return false;
    }

    Buffer.resize(*BytesRead);
    return true;
}

bool HttpClient::WriteData(std::vector<char> &BytesToSend)
{

#ifdef DEBUG
std::cout << "(" << socket_client << "): " << BytesToSend.size() << " bytes to send\n";
#endif

    int bytesSent = send(socket_client, &BytesToSend[0], BytesToSend.size(), 0);

#ifdef DEBUG
std::cout << "(" << socket_client << "): " << bytesSent << " bytes sent (of " << BytesToSend.size() << ")\n";
#endif

    if (bytesSent == SOCKET_ERROR)
    {
        isConnected = false;
        return false;
    }

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
        int bytesRead = recv(socket_client, &buffer[0], bufferSize, 0);
        if (bytesRead <= 0) break;
        else totalBytesRead += bytesRead;
    }
}

int HttpClient::BytesAvailable()
{
    u_long count;
    if (ioctlsocket(socket_client, FIONREAD, &count) < 0) {
        isConnected = false;
        /* look in WSAGetLastError() for the error code */
        return 0;
    }
    return (int)count;
}

void HttpClient::Die()
{
#ifdef DEBUG
std::cout << "(" << socket_client << "): Killing the poor guy\n";
#endif

    closesocket(socket_client);
}

#endif
