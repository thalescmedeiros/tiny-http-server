#include "HttpServerApp.h"
#include "HttpServerRoute.h"
#include "HttpServerControl.h"
#include "HttpBuilder.h"
#include "HttpRequest.h"
#include "HttpClientBase.h"
#include "HttpResponseFile.h"
#include "HttpWebSocket.h"
#include "TimerControl.h"

#define PAGE_START          101
#define PAGE_FILES          102
#define JSON_OPEN           201
#define JSON_CLOSE          202
#define WS_USERS_ONLINE     301
#define WS_ONLINE           302

HttpServerApp::HttpServerApp()
{
    allowFileServing = true;
    localFolder = "../www";

    routes.push_back(HttpServerRoute(JSON_OPEN, "/open", "GET", SERVER_COMMAND));
    routes.push_back(HttpServerRoute(JSON_CLOSE, "/close", "GET", SERVER_COMMAND));
    routes.push_back(HttpServerRoute(WS_USERS_ONLINE, "/users-online", "GET", WEB_SOCKET));
    routes.push_back(HttpServerRoute(WS_ONLINE, "/online", "GET", WEB_SOCKET));

    routes.push_back(HttpServerRoute(PAGE_START, "/", "/index.html", "GET", FRIENDLY_PATH));
    routes.push_back(HttpServerRoute(PAGE_FILES, "/*.html", "GET", FILE_FROM_SYSTEM));
    routes.push_back(HttpServerRoute(PAGE_FILES, "/img/*.*", "GET", FILE_FROM_SYSTEM));
    routes.push_back(HttpServerRoute(PAGE_FILES, "/css/*.css", "GET", FILE_FROM_SYSTEM));
    routes.push_back(HttpServerRoute(PAGE_FILES, "/js/*.js", "GET", FILE_FROM_SYSTEM));
}

bool HttpServerApp::GetResponseToRequest(std::unique_ptr<HttpRequest> &request, std::unique_ptr<HttpClientBase> &client)
{
    auto route = request->GetRoute();

    if (route->GetIdPage() == JSON_OPEN)
    {
        std::string json = "{\"Success\":true}";
        HttpBuilder::FromJson(json, client);
    }
    else if (route->GetIdPage() == JSON_CLOSE)
    {
        std::string json = "{\"Success\":true}";
        HttpBuilder::FromJson(json, client);
    }
    else return false;

    return true;
}

bool HttpServerApp::HandleWebSocket(std::unique_ptr<HttpClientBase> &client, CUSTOM_TIME timeStart)
{
    int id_route = client->GetCurrentRequest()->GetRoute()->GetIdPage();
    bool startConnection = client->GetCurrentState() == WEB_SOCKET_START;

    if (id_route == WS_USERS_ONLINE)
    {
        if (startConnection)
        {
            HttpWebSocket::SendTextMessage(client, client->GetServer()->GetOnlineUsersJson());
        }

        // Checking for any incoming text
        std::string textReceived;
        if (HttpWebSocket::ProcessIncomingMessage(client, textReceived))
        {
            // The NewTransaction keeps te client active
            client->NewTransaction();
        }

        // Checking if there's any message in the server to be sent
        int eventType;
        std::string textOut;
        if (client->GetServer()->HasWebSocketMessageToSend(eventType, textOut)
            && eventType == EVENT_WS_USERS_CHANGED)
            HttpWebSocket::SendTextMessage(client, textOut);
    }

    else if (id_route == WS_ONLINE)
    {
        if (startConnection)
        {
            HttpWebSocket::SendTextMessage(client, std::string("{\"Text\":\"oi\"}"));
        }

        // Checking for any incoming text
        std::string textReceived;
        if (HttpWebSocket::ProcessIncomingMessage(client, textReceived))
        {
            // The NewTransaction keeps te client active
            client->NewTransaction();

            //std::cout << "WS: \"" << textReceived << "\"\n";
        }

        // Checking if there's any message in the server to be sent
        int eventType;
        std::string textOut;
        if (client->GetServer()->HasWebSocketMessageToSend(eventType, textOut)
            && eventType != EVENT_WS_USERS_CHANGED)
            HttpWebSocket::SendTextMessage(client, textOut);
    }
    
    return true;
}
