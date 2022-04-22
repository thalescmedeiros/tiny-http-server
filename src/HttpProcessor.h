#ifndef H_HttpProcessor
#define H_HttpProcessor

#include <memory>
#include "TimerControl.h"

class HttpServerControl;
class HttpClientBase;
class HttpRequest;

class HttpProcessor
{
    public:
        HttpProcessor(std::unique_ptr<HttpServerControl> &server);
        void HandleRequest(std::unique_ptr<HttpClientBase> &client, CUSTOM_TIME timeNow);
        void KillClientWithResponse(std::unique_ptr<HttpClientBase> &client);

    private:
        bool RespondToRequest(std::unique_ptr<HttpClientBase> &client, CUSTOM_TIME timeStart, bool forceSendEverything);
        bool GetRequest(std::unique_ptr<HttpClientBase> &client, CUSTOM_TIME timeStart);
        bool IdentifyRequest(std::unique_ptr<HttpRequest> &request);
        void InitializeResponse(std::unique_ptr<HttpClientBase> &client);

        std::unique_ptr<HttpServerControl> &server;
};

#endif
