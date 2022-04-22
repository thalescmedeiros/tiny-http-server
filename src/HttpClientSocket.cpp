#ifndef __WIN64
#ifndef ARDUINO

//#define DEBUG
//#define ERRORS_ANALYSIS

#include <memory>
#include <vector>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <errno.h>
#include "HttpServerControl.h"
#include "HttpClientSocket.h"
#include "TimerControl.h"

HttpClient::HttpClient(std::unique_ptr<HttpServerControl> &server, int id_client_socket, std::string ipv4, int clientPort)
    : HttpClientBase(server, ipv4, clientPort)
{
    this->id_client_socket = id_client_socket;
    this->isConnected = true;
}

bool HttpClient::IsConnected() { return isConnected; }

bool HttpClient::ReadData(int NumBytes, std::vector<char> &Buffer, int *BytesRead)
{
    // Defining how many bytes we'll read (based on max buffer)
    int bytesToRead = HTTP_READ_BUFFER_SIZE > NumBytes ? NumBytes : HTTP_READ_BUFFER_SIZE;
    if (bytesToRead != NumBytes) Buffer.resize(bytesToRead);

    // Reading everything
    errno = 0;
    *BytesRead = recv(id_client_socket, &Buffer[0], bytesToRead, /*MSG_PEEK |*/ MSG_DONTWAIT);

#ifdef DEBUG
std::cout << "(" << id_client_socket << "): " << *BytesRead << " bytes received\n";
#endif
#ifdef ERRORS_ANALYSIS
int error_code;
socklen_t error_code_size = sizeof(error_code);
getsockopt(id_client_socket, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
std::cout << "Current Error: " << error_code << "\n";
#endif

    if (*BytesRead == -1) {
        if (errno == EBADF || errno == ENOTCONN) {
            std::cout << "(" << id_client_socket << "): killing during readData\n";
            isConnected = false;
        }

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
//std::cout << "(" << id_client_socket << "): " << BytesToSend.size() << " bytes to send\n";
#endif
#ifdef ERRORS_ANALYSIS
int error_code;
socklen_t error_code_size = sizeof(error_code);
getsockopt(id_client_socket, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
std::cout << "Current Error: " << error_code << "\n";
#endif

    errno = 0;
    int bytesSent = send(id_client_socket, &BytesToSend[0], BytesToSend.size(), 0);

#ifdef DEBUG
std::cout << "(" << id_client_socket << "): " << bytesSent << " bytes sent (of " << BytesToSend.size() << ")\n";

// std::cout << "\"";
// for (int i = 0; i < BytesToSend.size(); i++)
//     std::cout << BytesToSend[i];
// std::cout << "\"\n";

#endif
#ifdef ERRORS_ANALYSIS
error_code_size = sizeof(error_code);
getsockopt(id_client_socket, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
std::cout << "Current Error: " << error_code << "\n";
#endif

    //send(id_client_socket, &BytesToSend[0], BytesToSend.size(), 0);
    if (bytesSent == -1 || errno == EBADF || errno == ENOTCONN)
    {
        std::cout << "(" << id_client_socket << "): killing during WriteData\n";
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
        int bytesRead = recv(id_client_socket, &buffer[0], bufferSize, /*MSG_PEEK |*/ MSG_DONTWAIT);
        if (bytesRead <= 0) break;
        else totalBytesRead += bytesRead;
    }
}

int HttpClient::BytesAvailable()
{
    int count;
    errno = 0;
    ioctl(id_client_socket, FIONREAD, &count);
    if (errno == EBADF || errno == ENOTCONN) {
        std::cout << "(" << id_client_socket << "): killing during BytesAvailable\n";
        isConnected = false;
    }
    return count;
}

void HttpClient::Die()
{
#ifdef ERRORS_ANALYSIS
int error_code;
socklen_t error_code_size = sizeof(error_code);
getsockopt(id_client_socket, SOL_SOCKET, SO_ERROR, &error_code, &error_code_size);
std::cout << "Current Error: " << error_code << "\n";
#endif
#ifdef DEBUG
std::cout << "(" << id_client_socket << "): Killing the poor guy\n";
#endif

    errno = 0;
    shutdown(id_client_socket, SHUT_RDWR);
}

#endif
#endif
