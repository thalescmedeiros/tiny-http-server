#ifdef __WIN64

#ifndef H_HttpClientWinsock
#define H_HttpClientWinsock

#include <memory>
#include <vector>
#include <WinSock2.h>
#include "HttpClientBase.h"

class HttpServerControl;

class HttpClient : public HttpClientBase
{
    public:
        HttpClient(std::unique_ptr<HttpServerControl> &server, SOCKET socket_client, std::string ipv4, int clientPort);
        bool ReadData(int NumBytes, std::vector<char> &Buffer, int *BytesRead) override;
        bool WriteData(std::vector<char> &BytesToSend) override;
        void ClearReadBuffer() override;
        int BytesAvailable() override;
        bool IsConnected() override;
        void Die() override;

        int GetId() override { return socket_client; }
        SOCKET GetSocket() { return socket_client; }

    private:
        SOCKET socket_client;
        bool isConnected;
};

#endif

#endif
