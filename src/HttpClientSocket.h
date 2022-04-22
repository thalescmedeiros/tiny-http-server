#ifndef __WIN64
#ifndef ARDUINO

#ifndef H_HttpClientSocket
#define H_HttpClientSocket

#include <memory>
#include <vector>
#include "HttpClientBase.h"

class HttpServerControl;

class HttpClient : public HttpClientBase
{
    public:
        HttpClient(std::unique_ptr<HttpServerControl> &server, int id_client_socket, std::string ipv4, int clientPort);
        bool ReadData(int NumBytes, std::vector<char> &Buffer, int *BytesRead) override;
        bool WriteData(std::vector<char> &BytesToSend) override;
        void ClearReadBuffer() override;
        int BytesAvailable() override;
        bool IsConnected() override;
        void Die() override;

        int GetId() override { return id_client_socket; }

    private:
        int id_client_socket;
        bool isConnected;
};

#endif

#endif
#endif
