#ifdef ARDUINO

#ifndef H_HttpClientArduino
#define H_HttpClientArduino

#include <WiFiNINA.h>
#include <memory>
#include <vector>
#include <string>
#include "HttpClientBase.h"

class HttpServerControl;

class HttpClient : public HttpClientBase
{
    public:
        HttpClient(std::unique_ptr<HttpServerControl> &server, WiFiClient client, std::string ipv4, int clientPort);
        int GetClient();
        bool ReadData(int NumBytes, std::vector<unsigned char> &Buffer, int *BytesRead) override;
        bool WriteData(std::vector<char> &BytesToSend) override;
        void ClearReadBuffer() override;
        int BytesAvailable() override;
        bool IsConnected() override;
        void Die() override;

    private:
        WiFiClient client;
};

#endif

#endif
