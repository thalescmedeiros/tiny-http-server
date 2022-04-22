#ifdef ARDUINO

#ifndef H_HttpServerArduino
#define H_HttpServerArduino

#include <WiFiNINA.h>
#include "HttpServerControl.h"
#include "HttpClient.h"

class HttpServer
{
    public:
        HttpServer() { }
        void Start();
        void Stop();
        bool CheckForNewClients(std::unique_ptr<HttpServerControl> &server, std::unique_ptr<HttpConnection> *NewClient);

    private:
        WiFiServer wifiServer(80);
};

#endif

#endif
