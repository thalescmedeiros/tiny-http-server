#include <iostream>
#include <memory>
#include "HttpClient.h"
#include "HttpClientBase.h"
#include "TimerControl.h"
#include "HttpServerControl.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

HttpClientBase::HttpClientBase(std::unique_ptr<HttpServerControl> &server, std::string ipv4, int clientPort)
    : server(server)
{
    this->timeStartConnection = GetCustomCurrentTime();
    this->currentState = IDLE;
    this->numberTransactions = 0;
    this->mustClose = false;
    this->ipv4 = ipv4;
    this->clientPortNumber = clientPort;
}

// If the client wants a keep-alive connection
// and we want to keep it, we use the mustClose
// parameter to keep track of should or not close
// the connection after the response
bool HttpClientBase::GetMustClose() { return mustClose; }
void HttpClientBase::SetMustClose(bool mustClose) { this->mustClose = mustClose; }

// Every time a new request is received by the client,
// we record as "new transaction", keeping track of how
// long ago the client is idle
void HttpClientBase::NewTransaction()
{
    this->numberTransactions++;
    this->timeLastTransaction = GetCustomCurrentTime();
    
    // Generating event for change in connected users
    server->UsersUpdated();
}
CUSTOM_TIME HttpClientBase::GetTimeStartConnection() { return timeStartConnection; }
CUSTOM_TIME HttpClientBase::GetTimeLastTransaction() { return timeLastTransaction; }
int HttpClientBase::GetNumberTransactions() { return this->numberTransactions; }
double HttpClientBase::GetInactiveTime() { return TIME_DIFF_TO_MSECS_DOUBLE((TIME_DIFF)(GetCustomCurrentTime() - (numberTransactions == 0 ? this->timeStartConnection : this->timeLastTransaction))); }

// Being active means OPEN connection and not timed out for inactivity
bool HttpClientBase::IsActive()
{
    return
        currentState != CLOSING_CONNECTION
        && this->IsConnected()
        #if MAX_TIMEOUT_DISCONNECT != 0
        && this->GetInactiveTime() < MAX_TIMEOUT_DISCONNECT
        #endif
        ;
}

// The methods below refer to the request being received
// and the response to be sent to the client
void HttpClientBase::StartReceivingNewRequest() { currentRequest.reset(new HttpRequest()); }
std::unique_ptr<HttpRequest> &HttpClientBase::GetCurrentRequest() { return currentRequest; }
std::unique_ptr<HttpResponse> &HttpClientBase::GetCurrentResponse() { return currentResponse; }
std::unique_ptr<HttpServerControl> &HttpClientBase::GetServer() { return server; }
void HttpClientBase::SetCurrentResponse(std::unique_ptr<HttpResponse> &response)
{
    HttpResponse *r = response.get();
    response.release();
    currentResponse.reset(r);
}

// State represents if the client is idle, sending request
// or if we are already dealing with the response to it
CLIENT_STATE HttpClientBase::GetCurrentState() { return currentState; }
void HttpClientBase::SetState(CLIENT_STATE newState) { this->currentState = newState; }

void Tick();

// The methods below will come from the platform in use
bool HttpClientBase::AvailableData() { return BytesAvailable() > 0; }

void HttpClientBase::Tick()
{
    // Some action going on?
    if ((currentState == RECEIVING_REQUEST && this->AvailableData())
        || currentState == SENDING_RESPONSE
        || currentState == WEB_SOCKET_START
        || currentState == WEB_SOCKET_ALIVE
        || (currentState == IDLE && this->AvailableData()))
    {
        auto client = std::unique_ptr<HttpClientBase>(this);
        auto timeNow = GetCustomCurrentTime();

        server->GetProcessor()->HandleRequest(client, timeNow);
        
        // Making sure the destructor of "this" will not be called
        client.release();
    }
    
    // If state is to close connection, close socket smoothly
    if (currentState == CLOSING_CONNECTION)
        this->Die();
}
