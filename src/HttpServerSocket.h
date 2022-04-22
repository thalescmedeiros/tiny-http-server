#ifndef __WIN64
#ifndef ARDUINO

#ifndef H_HttpServerSocket
#define H_HttpServerSocket

#include <memory>

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
        int id_server_socket;
};

#endif

#endif
#endif
