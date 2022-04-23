# tiny-http-server
Tiny HTTP Server written in C++, capable of handling internal commands, file serving and WebSocket clients.

# About The Project

The scope of this project is to provide an extremely high performance server which securely receive HTTP simple commands which can be customized by the developer for any sort of application which are STL compatible (Windows, Linux, Arduino, etc).

Security without HTTPS:

- With concept of Routes, only requests that match the available routes will be accepted;
- Routes are checked before accessing the file system, preventing path string injection;
- Extended capability of authenticating every user (BASIC, NTLM and custom);

## Built With

The language used for this project is C++ (version 14), built with STL libraries and platform specifics.

# Getting Started

## Prerequisities

STL:
- MEMORY
- ALGORITHM
- VECTOR
- STRING
- THREAD
- IOSTREAM

Platform specifics:
- Winsock2 (Windows)
- sys/socket (Linux/Unix based)
- WiFiNina (Arduino)

Pre-compiler directives are placed in the code that will perform the proper includes for each platform.

## Installation

1. Clone the repo
`git clone https://github.com/thalescmedeiros/tiny-http-server.git`
3. Customize your HttpServerApp.cpp
```C++
// Giving some unique numbers
// for the routes to be identified
#define PAGE_START          101
#define PAGE_FILES          102
#define JSON_OPEN           201
#define JSON_CLOSE          202

HttpServerApp::HttpServerApp()
{
    allowFileServing = true;
    localFolder = "../www";

    routes.push_back(HttpServerRoute(JSON_OPEN, "/open", "GET", false, SERVER_COMMAND));
    routes.push_back(HttpServerRoute(JSON_CLOSE, "/close", "GET", false, SERVER_COMMAND));

    routes.push_back(HttpServerRoute(PAGE_START, "/", "/index.html", "GET", false, FRIENDLY_PATH));
    routes.push_back(HttpServerRoute(PAGE_FILES, "/*.html", "GET", false, FILE_FROM_SYSTEM));
    routes.push_back(HttpServerRoute(PAGE_FILES, "/img/*.*", "GET", false, FILE_FROM_SYSTEM));
}

bool HttpServerApp::GetResponseToRequest(std::unique_ptr<HttpRequest> &request, std::unique_ptr<HttpClientBase> &client)
{
    auto route = request->GetRoute();

    if (route->GetIdPage() == JSON_OPEN)
    {
        //----------------------------
        // Do some fance stuff here,
        // then send the response to
        // the client accordingly
        //----------------------------
        std::string json = "{\"Success\":true}";
        HttpBuilder::FromJson(json, client);
    }
}

bool HttpServerApp::HandleWebSocket(std::unique_ptr<HttpClientBase> &client, CUSTOM_TIME timeStart)
{
    // Checking for any incoming text
    std::string textReceived;
    if (HttpWebSocket::ProcessIncomingMessage(client, textReceived))
    {
        // The NewTransaction keeps the client active
        client->NewTransaction();
    }
}
```
3. Build and run

# Examples

1. Simple application serving only files
```C++
// Giving some unique numbers
// for the routes to be identified
#define PAGE_START          101
#define PAGE_FILES          102

HttpServerApp::HttpServerApp()
{
    allowFileServing = true;
    localFolder = "../www";

    routes.push_back(HttpServerRoute(PAGE_START, "/", "/index.html", "GET", false, FRIENDLY_PATH));
    routes.push_back(HttpServerRoute(PAGE_FILES, "/*.html", "GET", false, FILE_FROM_SYSTEM));
    routes.push_back(HttpServerRoute(PAGE_FILES, "/img/*.*", "GET", false, FILE_FROM_SYSTEM));
}
```

2. WebSocket for online/connected users

The current web page in the source code already contains an example of WebSocket for checking the users online from the server. The list of connected users and their last Request to the server is built-in the server.

The server automatically generates an event for sending to all WS clients a refreshed list of users (JSON format), every time a new request is performed. However, the application shall treat this in the specified route:

```C++
#define WS_USERS_ONLINE     301

HttpServerApp::HttpServerApp()
{
    //...
    routes.push_back(HttpServerRoute(WS_USERS_ONLINE, "/users-online", false, "GET", WEB_SOCKET));
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
            // The NewTransaction keeps the client active
            client->NewTransaction();
        }

        // Checking if there's any message in the server to be sent
        int eventType;
        std::string textOut;
        if (client->GetServer()->HasWebSocketMessageToSend(eventType, textOut)
            && eventType == EVENT_WS_USERS_CHANGED)
            HttpWebSocket::SendTextMessage(client, textOut);
    }
}
```

Result in the webpage with all information from users (the list is updated in real time):

![index.html screenshot](https://user-images.githubusercontent.com/22152504/164890411-70b66db9-5b76-4dd6-998a-c58495359270.png)

3. WebSocket for custom application

Another WebSocket already in the source code page "/online" is receiving a JSON message from the server every 5 seconds. This code is implemented in the current main.cpp, but can be customized by the developer to the project's needs.

`main.cpp`
```C++
int main(int argc, char const *argv[])
{
    auto httpServer = std::unique_ptr<HttpServerControl>(new HttpServerControl());
    httpServer->Initialize();

    CUSTOM_TIME lastMessageSent = GetCustomCurrentTime();
    while (true)
    {
        // Mandatory TICK to handle everything
        httpServer->Tick();

        // Sleeping for 10ms
        std::this_thread::sleep_for(std::chrono::milliseconds(10));

        // Adding a message to be sent to all WebSockets every 5 seconds
        CUSTOM_TIME timeNow = GetCustomCurrentTime();
        double timeEllapsed = TIME_DIFF_TO_MSECS_DOUBLE(timeNow - lastMessageSent);
        if (timeEllapsed > 5000)
        {
            lastMessageSent = timeNow;
            httpServer->SendTextToWSClients(EVENT_WS_CUSTOM_1, std::string("{\"Text\":\"hi again!\"}"));
        }
    }

    return 0;
}
```

`HttpServerApp.cpp`
```C++
#define WS_ONLINE           302

HttpServerApp::HttpServerApp()
{
    //...
    routes.push_back(HttpServerRoute(WS_ONLINE, "/online", "GET", false, WEB_SOCKET));
}

bool HttpServerApp::HandleWebSocket(std::unique_ptr<HttpClientBase> &client, CUSTOM_TIME timeStart)
{
    int id_route = client->GetCurrentRequest()->GetRoute()->GetIdPage();
    bool startConnection = client->GetCurrentState() == WEB_SOCKET_START;

    if (id_route == WS_ONLINE)
    {
        if (startConnection)
        {
            HttpWebSocket::SendTextMessage(client, std::string("{\"Text\":\"First hi!\"}"));
        }

        // Checking for any incoming text from the client
        std::string textReceived;
        if (HttpWebSocket::ProcessIncomingMessage(client, textReceived))
        {
            // The NewTransaction keeps the client active
            client->NewTransaction();
        }

        // Checking if there's any message in the server to be sent
        int eventType;
        std::string textOut;
        if (client->GetServer()->HasWebSocketMessageToSend(eventType, textOut)
            && eventType == EVENT_WS_CUSTOM_1)
            HttpWebSocket::SendTextMessage(client, textOut);
    }
}
```

# Roadmap
- [x] Basic HTTP Server
- [x] WebSocket handling
- [ ] Users Authentication
- [ ] NTLM Authentication

# License
Distributed under the Distributed under the MIT License Version 2. See `LICENSE.txt` for more information.

# Contact
Thales Medeiros - thalescmedeiros@gmail.com

Project Link: https://github.com/thalescmedeiros/tiny-http-server/
