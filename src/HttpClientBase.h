#ifndef H_HttpClientBase
#define H_HttpClientBase

#include <iostream>
#include <vector>
#include <memory>
#include "TimerControl.h"
#include "HttpRequest.h"
#include "HttpResponse.h"

enum CLIENT_STATE {
    RECEIVING_REQUEST = 1,
    SENDING_RESPONSE,
    CLOSING_CONNECTION,
    WEB_SOCKET_START,
    WEB_SOCKET_ALIVE,
    IDLE
};

class HttpClientBase;
class HttpServerControl;
class HttpRequest;
class HttpResponse;
class HttpServerRoute;

class HttpClientBase
{
    public:
        HttpClientBase(std::unique_ptr<HttpServerControl> &server, std::string ipv4, int clientPort);

        // If the client wants a keep-alive connection
        // and we want to keep it, we use the mustClose
        // parameter to keep track of should or not close
        // the connection after the response
        bool GetMustClose();
        void SetMustClose(bool mustClose);

        // Every time a new request is received by the client,
        // we record as "new transaction", keeping track of how
        // long ago the client is idle
        void NewTransaction();
    	CUSTOM_TIME GetTimeStartConnection();
	    CUSTOM_TIME GetTimeLastTransaction();
        int GetNumberTransactions();
        double GetInactiveTime();

        // Being active means OPEN connection and not timed out for inactivity
        bool IsActive();
        
        // The methods below refer to the request being received
        // and the response to be sent to the client
        void StartReceivingNewRequest();
        std::unique_ptr<HttpRequest> &GetCurrentRequest();
        std::unique_ptr<HttpResponse> &GetCurrentResponse();
        std::unique_ptr<HttpServerControl> &GetServer();
        void SetCurrentResponse(std::unique_ptr<HttpResponse> &response);

        // State represents if the client is idle, sending request
        // or if we are already dealing with the response to it
        CLIENT_STATE GetCurrentState();
        void SetState(CLIENT_STATE newState);

        void Tick();

        // The methods below will come from the platform in use
        virtual bool ReadData(int NumBytes, std::vector<char> &Buffer, int *BytesRead) { return false; }
        virtual bool WriteData(std::vector<char> &BytesToSend) { return true; }
        virtual void ClearReadBuffer() { }
        bool AvailableData();
        virtual int GetId() { return 0; }
        virtual std::string GetIPV4Address() { return ipv4; }
        virtual std::string GetIPV6Address() { return ipv6; }
        virtual int GetClientPortNumber() { return clientPortNumber; }
        virtual int BytesAvailable() { return 0; }
        virtual bool IsConnected() { return true; }
        virtual void Die() { }
    private:
        std::unique_ptr<HttpServerControl> &server;
        std::unique_ptr<HttpRequest> currentRequest;
        std::unique_ptr<HttpResponse> currentResponse;
        std::unique_ptr<HttpServerRoute> currentRoute;
        CLIENT_STATE currentState;
        CUSTOM_TIME timeStartConnection;
	    CUSTOM_TIME timeLastTransaction;
        bool mustClose;
        int numberTransactions;
        std::string ipv4;
        std::string ipv6;
        int clientPortNumber;
};

#endif
