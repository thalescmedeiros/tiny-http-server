#include <memory>
#include <iostream>
#include <algorithm>
#include <string>
#include <sstream>
#include "GlobalSettings.h"
#include "HttpServerControl.h"
#include "HttpServerApp.h"
#include "HttpBuilder.h"
#include "HttpProcessor.h"
#include "HttpClientBase.h"
#include "HttpResponseFile.h"
#include "TimerControl.h"

HttpProcessor::HttpProcessor(std::unique_ptr<HttpServerControl> &server)
    : server(server) { }

void HttpProcessor::HandleRequest(std::unique_ptr<HttpClientBase> &client, CUSTOM_TIME timeStart)
{
    //----------------------------------------------------
    // LOGIC:
    // HandleRequest will be called as many times as it
    // needs, before the client times out (controlled by
    // HttpServerControl::Tick).
    // Every time this is called for the client, it tries
    // to execute a portion of the current step within the
    // time defined in MAX_PROCESSING_TIME.
    // EXAMPLE:
    // It will start receiving packets during RECEIVING
    // until it finishes receiving everything or until
    // it achieves the maximum amount of processing time.
    //----------------------------------------------------

    // If client was IDLE, start receiving new request
    double timeEllapsed = 0;
    if (client->GetCurrentState() == IDLE)
    {
        client->NewTransaction();
        client->StartReceivingNewRequest();
        client->SetState(RECEIVING_REQUEST);
    }

    // Receiving request
    timeEllapsed = TIME_DIFF_TO_MSECS_DOUBLE(GetCustomCurrentTime() - timeStart);
    if (client->GetCurrentState() == RECEIVING_REQUEST && timeEllapsed < MAX_PROCESSING_TIME)
    {
        // If receiving new request, check if we finish receiving
        // Then change state to sending response
        if (this->GetRequest(client, timeStart))
        {
            // If we finish receiving, let's identify the request
            this->IdentifyRequest(client->GetCurrentRequest());

            // DEBUG
            std::cout << "(" << client->GetId() << "): " << client->GetCurrentRequest()->GetMethod() << " " << client->GetCurrentRequest()->GetPath() << "\n";

            // And fill in the response object with starting data
            this->InitializeResponse(client);

            client->SetState(SENDING_RESPONSE);
        }
    }

    // Sending response
    timeEllapsed = TIME_DIFF_TO_MSECS_DOUBLE(GetCustomCurrentTime() - timeStart);
    if (client->GetCurrentState() == SENDING_RESPONSE && timeEllapsed < MAX_PROCESSING_TIME)
    {
        // If sending response, check if finish sending
        if (this->RespondToRequest(client, timeStart, false))
        {
            // Either close connection or set to idle
            if (client->GetMustClose())
            {
                client->SetState(CLOSING_CONNECTION);
            }
            else
            {
                auto route = client->GetCurrentRequest()->GetRoute();
                if (route != NULL && route->GetRequestType() == WEB_SOCKET)
                    client->SetState(WEB_SOCKET_START);
                else
                    client->SetState(IDLE);
            }
        }
    }

    // WebSocket handling
    timeEllapsed = TIME_DIFF_TO_MSECS_DOUBLE(GetCustomCurrentTime() - timeStart);
    if ((client->GetCurrentState() == WEB_SOCKET_START || client->GetCurrentState() == WEB_SOCKET_ALIVE) && timeEllapsed < MAX_PROCESSING_TIME)
    {
        // Switching for CLOSING if the handle returns false
        if (!server->GetApp()->HandleWebSocket(client, GetCustomCurrentTime()))
            client->SetState(CLOSING_CONNECTION);
        // Switching to ALIVE after the first action
        else if (client->GetCurrentState() == WEB_SOCKET_START)
            client->SetState(WEB_SOCKET_ALIVE);
    }
}

bool HttpProcessor::GetRequest(std::unique_ptr<HttpClientBase> &client, CUSTOM_TIME timeStart)
{
    std::unique_ptr<HttpRequest> &request = client->GetCurrentRequest();

    int availableBytes = client->BytesAvailable();
    auto content = request->GetContent();

    //-------------------------------------------------
    // If the client is abusing with packets, kill him!
    if (content.size() + availableBytes >= MAX_HTTP_REQUEST_SIZE)
    {
        client->SetState(CLOSING_CONNECTION);
        return false;
    }
    //-------------------------------------------------

    // First, let's get all available data
    auto buffer = std::vector<char>(availableBytes);

    int numBytesRead;
    if (!client->ReadData(availableBytes, buffer, &numBytesRead))
        // If we didn't receive data, leave
        return false;

    // Inserting the received buffer to the end of current content
    content.insert(content.end(), buffer.begin(), buffer.end());

    //-------------------------------------------------
    // HEADERS
    //-------------------------------------------------
    // First receival, if we didn't get a first line yet
    if (request->GetMethod().empty())
    {
        std::string crlf1 = "\r\n";
        std::string crlf2 = "\r\n\r\n";
        std::string protocol = "HTTP/1.1";

        // Waiting for the end of headers section
        auto itEndHeaders = std::search(content.begin(), content.end(), crlf2.begin(), crlf2.end());
        // If we didn't finish receiving the header, leave
        if (itEndHeaders == content.end()) return false;

        // Filling first line to string
        auto itEndFirstLine = std::search(content.begin(), itEndHeaders, crlf1.begin(), crlf1.end());
        std::string firstLine = std::string(content.begin(), itEndFirstLine);

        // Decoding first line into method/path and checking protocol string
        std::istringstream f(firstLine);
        std::string s;
        int field = 0;
        while (std::getline(f, s, ' ')) {
            field++;
            if (field > 3 || (field == 3 && s.compare(protocol) != 0)) {
                // Invalid request!!!
                client->SetState(CLOSING_CONNECTION);
                return false;
            }

            if (field == 1) request->SetMethod(s);
            if (field == 2) request->SetPath(s);
        }

        // Copying the rest of the content to the headers
        std::advance(itEndFirstLine, crlf1.size());
        std::string headers = std::string(itEndFirstLine, itEndHeaders);
        request->SetHeaders(headers);

        // Erasing the beginning of the content
        std::advance(itEndHeaders, crlf2.size());
        content.erase(content.begin(), itEndHeaders);
    }

    //-------------------------------------------------
    // CONTENT
    //-------------------------------------------------
    std::string headerLength = "Content-Length";
    auto valueLength = request->GetHeader(headerLength);
    int contentLength = 0;
    if (!valueLength.empty())
    {
        try { contentLength = std::stoi(valueLength); }
        catch (...)
        {
            // Invalid Content-Length header!!!
            client->SetState(CLOSING_CONNECTION);
            return false;
        }
    }

    // If the current content size is equals to the header Content-Length
    // then we finish reading the request and will return TRUE
    return content.size() >= contentLength;
}

bool HttpProcessor::IdentifyRequest(std::unique_ptr<HttpRequest> &request)
{
    // Looping through all server routes and check if
    // there's any internal route that corresponds to
    // the requested METHOD / PATH
    std::vector<HttpServerRoute> &routes = server->GetApp()->GetRoutes();
    std::vector<HttpServerRoute>::iterator it = routes.begin();
    int i = 0;
    while (it != routes.end())
    {
        HttpServerRoute *route = &(*it);
        //-----------------------------------------------
        // INTERNAL COMMANDS
        //-----------------------------------------------
        if ((route->GetRequestType() == SERVER_COMMAND || route->GetRequestType() == WEB_SOCKET)
            
            && request->GetPath().compare(route->GetUrl()) == 0
            //&& route->PathMatchesUrlPattern(request->GetPath())
            && request->GetMethod().compare(route->GetMethod()) == 0)
        {
            request->SetRoute(route);
            return true;
        }

        //-----------------------------------------------
        // FILE OUTPUT
        //-----------------------------------------------
        if ((route->GetRequestType() == FILE_FROM_SYSTEM || route->GetRequestType() == FRIENDLY_PATH)
            && server->GetApp()->GetAllowFileServing()
            && request->GetMethod().compare(route->GetMethod()) == 0
            && route->PathMatchesUrlPattern(request->GetPath()))
        {
            request->SetRoute(route);
            return true;
        }

        it++;
    }
    return false;
}

void HttpProcessor::InitializeResponse(std::unique_ptr<HttpClientBase> &client)
{
    std::unique_ptr<HttpRequest> &request = client->GetCurrentRequest();
    HttpServerRoute *route = request->GetRoute();

    if (route == NULL)
    {
        HttpBuilder::NotFound(client);
    }
    else if (route->GetRequestType() == WEB_SOCKET)
    {
        HttpBuilder::WebSocketStartResponse(client);
    }
    else if (route->GetRequestType() == SERVER_COMMAND)
    {
        server->GetApp()->GetResponseToRequest(request, client);
    }
    else // File request
    {
        std::unique_ptr<HttpResponseFile> response(new HttpResponseFile());
        if (!response->FileExists(server->GetApp()->GetLocalFolder(), route->GetRequestType() == FILE_FROM_SYSTEM ? request->GetPath() : route->GetPathMapped()))
            HttpBuilder::NotFound(client);
        else
        {
            response->InitializeResponse(client);
            client->SetCurrentResponse((std::unique_ptr<HttpResponse>&)std::move(response));
        }
    }
}

bool HttpProcessor::RespondToRequest(std::unique_ptr<HttpClientBase> &client, CUSTOM_TIME timeStart, bool forceSendEverything)
{
    std::unique_ptr<HttpRequest> &request = client->GetCurrentRequest();
    std::unique_ptr<HttpResponse> &response = client->GetCurrentResponse();

    //----------------------------------------
    // IMPORTANT!!! Request can be null, when
    // server is out of resources and wants
    // to kick the client quickly.
    //----------------------------------------

    // Sending protocol line and headers
    if (!response->GetHeadersSent())
    {
        // Checking if we shall add Connection header
        std::string headerConnection = "Connection";
        std::string keepAliveValue = "keep-alive";
        std::string upgradeValue = "upgrade";
        // Check first if request exists
        std::string connectionValue = !request ? "" : request->GetHeader(headerConnection);
        std::transform(connectionValue.begin(), connectionValue.end(), connectionValue.begin(), [](unsigned char c) { return std::tolower(c); });
        bool keepAlive = connectionValue.compare(keepAliveValue) == 0;
        bool upgrading = connectionValue.compare(upgradeValue) == 0;
        if (!response->ContainsHeader(headerConnection))
        {
            response->AddHeader(headerConnection, keepAlive ? "keep-alive" : "close");
        }
        // Adding the content-length header
        if (response->GetContentLength() > 0)
            response->AddHeader("Content-Length", std::to_string(response->GetContentLength()));
        if (!keepAlive && !upgrading)
        {
            client->SetMustClose(true);
        }

        // First line with protocol and status
        std::string dataToSend = "HTTP " + std::to_string(response->GetStatusCode()) + " " + response->GetReasonPhrase() + "\r\n";
        
        // Adding headers
        dataToSend += response->GetHeaders();

        // Finishing the double line for header ending
        dataToSend += "\r\n";

        // Arranging everything in a vector and sending
        auto arrData = std::vector<char>(dataToSend.begin(), dataToSend.end());
        client->WriteData(arrData);

        response->SetHeadersSent(true);

        // Checking if time expired after sending the header
        double timeEllapsed = TIME_DIFF_TO_MSECS_DOUBLE(GetCustomCurrentTime() - timeStart);
        if (!forceSendEverything && timeEllapsed >= MAX_PROCESSING_TIME && response->GetContentLength() > 0) return false;
    }

    auto route = !request ? NULL : request->GetRoute();
    if (route == NULL || route->GetRequestType() == SERVER_COMMAND || route->GetRequestType() == WEB_SOCKET)
    {
        if (response->GetContentLength() > 0)
        {
            // Writing at once everything, if it's an internal command
            auto content = response->GetContent();
            client->WriteData(content);
        }
        return true;
    }

    else
    {
        // The metod below writes the file until
        // MAX_PROCESSING_TIME is reached
        HttpResponseFile* resp = ((HttpResponseFile*)response.get());
        bool result = resp->WriteFileToClient(client, timeStart);
        return result;
    }

    return true;
}

void HttpProcessor::KillClientWithResponse(std::unique_ptr<HttpClientBase> &client)
{
    client->ClearReadBuffer();
    this->RespondToRequest(client, GetCustomCurrentTime(), true);
    client->Die();
}
