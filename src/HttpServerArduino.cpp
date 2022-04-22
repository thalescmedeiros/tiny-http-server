#ifdef ARDUINO

#include "HttpServerArduino.h"

void HttpServer::Start()
{
    wifiServer.begin();
}

void HttpServer::Stop()
{
    wifiServer.end();
}

bool HttpServer::CheckForNewClients(std::unique_ptr<HttpServerControl> &server, std::unique_ptr<HttpConnection> *NewClient)
{
    WiFiClient client = wifiServer.available();
    if (client) {
        NewClient->reset(new HttpConnection(server, client));
        return true;
    }
    else return false;
}

#endif
