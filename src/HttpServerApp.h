#ifndef H_HttpServerApp
#define H_HttpServerApp

#include <memory>
#include <vector>
#include <string>
#include "TimerControl.h"

class HttpRequest;
class HttpClientBase;
class HttpServerRoute;

class HttpServerApp
{
    public:
        HttpServerApp();
        
        bool IdentifyRequest(std::unique_ptr<HttpRequest> &request);
        virtual bool GetResponseToRequest(std::unique_ptr<HttpRequest> &request, std::unique_ptr<HttpClientBase> &client) { }
        virtual bool HandleWebSocket(std::unique_ptr<HttpClientBase> &client, CUSTOM_TIME timeStart) { }

        std::vector<HttpServerRoute> &GetRoutes() { return routes; }
        bool GetAllowFileServing() { return allowFileServing; }
        std::string GetLocalFolder() { return localFolder; }
        bool GetIsNTLMActive() { return isNTLMActive; }

    private:
        std::vector<HttpServerRoute> routes;
        bool allowFileServing;
        std::string localFolder;
        bool isNTLMActive;
};

#endif
