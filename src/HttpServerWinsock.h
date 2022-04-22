#ifdef __WIN64

#ifndef H_HttpServerWinsock
#define H_HttpServerWinsock

#include <memory>
#include <WinSock2.h>

class HttpServerControl;
class HttpClient;

class HttpServer
{
    public:
        HttpServer() { }
        void Start();
        void Stop();
        bool CheckForNewClients(std::unique_ptr<HttpServerControl> &server, std::unique_ptr<HttpClient> &client);

    private:
        SOCKET socket_server;
};

#endif

#endif