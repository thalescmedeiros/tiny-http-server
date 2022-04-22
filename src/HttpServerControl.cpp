#include <memory>
#include <iostream>
#include <algorithm>

#include "HttpServer.h"
#include "HttpClient.h"
#include "HttpBuilder.h"
#include "HttpServerControl.h"

HttpServerControl::HttpServerControl()
{
    serverControl = std::unique_ptr<HttpServerControl>(this);
    server = std::unique_ptr<HttpServer>(new HttpServer());
    serverApp = std::unique_ptr<HttpServerApp>(new HttpServerApp());
    processor.reset(new HttpProcessor(serverControl));
}

HttpServerControl::~HttpServerControl()
{
    // Releasing self reference
    serverControl.release();

    // Closing any connected client
    if (!clients.empty())
    {
        // for (int i = 0; i < clients.size(); i++)
        //     clients[i]->Die();
        for (const auto &client : clients)
            client.get()->Die();

        clients.clear();
    }

    // Stopping the server
    server->Stop();
}

void HttpServerControl::Initialize()
{
    server->Start();
}

bool HttpServerControl::AddClient(std::unique_ptr<HttpClient> &client)
{
    int clientsConnected = 0;
    bool added = false;

    if (clients.size() >= MAX_HTTP_CLIENTS)
    {
        // Not enough memory
        return false;
    }

    clients.push_back(std::move(client));

    // Generating event for change in connected users
    this->SendTextToWSClients(EVENT_WS_USERS_CHANGED, this->GetOnlineUsersJson());

    return true;
}

void HttpServerControl::Tick()
{
    std::unique_ptr<HttpClient> newClient;
    if (server->CheckForNewClients(serverControl, newClient))
        if (!AddClient(newClient))
        {
            HttpBuilder::ServerTooBusy((std::unique_ptr<HttpClientBase>&)newClient);
            processor->KillClientWithResponse((std::unique_ptr<HttpClientBase>&)newClient);
        }

    // Iterate over the list using Iterators and erase elements
    // which are multiple of 3 while iterating through list
    auto it = clients.begin();

    while (it != clients.end()) {
        // Checking if client is active (still connected and
        // not timed out since last activity)
        if (!it->get()->IsActive()) {
            // Remove elements while iterating
            // erase() makes the passed iterator invalid
            // But returns the iterator to the next of deleted element
            it->get()->Die();
            it = clients.erase(it);
        } else {
            it->get()->Tick();
            it++;
        }
    }

    // Here all WS clients received the first message
    // on queue. It's safe to remove it, then
    if (messagesToClientWSText.size() > 0)
    {
        messagesToClientWSEventType.erase(messagesToClientWSEventType.begin());
        messagesToClientWSText.erase(messagesToClientWSText.begin());
    }
}

void HttpServerControl::RemoveClient(std::unique_ptr<HttpClient> &client)
{
    clients.erase(std::remove(clients.begin(), clients.end(), client), clients.end());

    // Generating event for change in connected users
    this->SendTextToWSClients(EVENT_WS_USERS_CHANGED, this->GetOnlineUsersJson());
}

void HttpServerControl::SendTextToWSClients(int eventType, std::string textOut)
{
    messagesToClientWSEventType.push_back(eventType);
    messagesToClientWSText.push_back(textOut);
}

bool HttpServerControl::HasWebSocketMessageToSend(int &eventType, std::string &textOut)
{
    if (messagesToClientWSText.size() > 0)
    {
        eventType = messagesToClientWSEventType[0];
        textOut = messagesToClientWSText[0];
        return true;
    }
    
    return false;
}

std::string HttpServerControl::GetOnlineUsersJson()
{
    std::string json = "{\"Users\":[";
    bool first = true;
    for (auto & client : clients)
    {
        if (first) first = false;
        else json += ",";

        json += "{";
        json += "\"Id\":" + std::to_string(client->GetId());
        json += ",\"IPV4Address\":\"" + client->GetIPV4Address() + "\"";
        json += ",\"ClientPort\":" + std::to_string(client->GetClientPortNumber());
        json += "}";
    }
    json += "]}";
    return json;
}
