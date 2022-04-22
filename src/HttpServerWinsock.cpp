#ifdef __WIN64

#include <iostream>
#include <string>
#include <winsock2.h>
#include "GlobalSettings.h"
#include "HttpServerWinsock.h"
#include "HttpClientWinsock.h"

void HttpServer::Start()
{
    const int iReqWinsockVer = 2;   // Minimum winsock version required
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(iReqWinsockVer,0), &wsaData) != 0)
    {
        // startup failed
        perror("Server startup failed");
        exit(EXIT_FAILURE);
    }

    // Check if major version is at least iReqWinsockVer
    if (LOBYTE(wsaData.wVersion) < iReqWinsockVer)
    {
        perror("Winsock2 not available");
        exit(EXIT_FAILURE);
    }
    
    // Creating the server socket
    socket_server = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    if (socket_server == INVALID_SOCKET)
    {
        perror("Server socket failed to be created");
        exit(EXIT_FAILURE);
    }

    sockaddr_in sockAddr;
    sockAddr.sin_family = AF_INET;
    sockAddr.sin_port = htons(HTTP_PORT);
    sockAddr.sin_addr.S_un.S_addr = INADDR_ANY; // use default

    // Bind socket to port
    if (bind(socket_server, (sockaddr*)(&sockAddr), sizeof(sockAddr)) != 0)
    {
        perror("HTTP port is not available for binding");
        exit(EXIT_FAILURE);
    }

    // Puts a socket in the listening state
    if (listen(socket_server, MAX_HTTP_CLIENTS) != 0)
    {
        perror("Not able to put the socket in listening state");
        exit(EXIT_FAILURE);
    }

    // Change the socket mode on the listening socket from blocking to
    // non-block so the application will not block waiting for requests
    ULONG NonBlock = 1;
    if (ioctlsocket(socket_server, FIONBIO, &NonBlock) == SOCKET_ERROR)
    {
        perror("Not able to put the socket in non-blocking mode");
        exit(EXIT_FAILURE);
    }
}

void HttpServer::Stop()
{
    closesocket(socket_server);

    // Cleanup winsock
    if (WSACleanup() != 0)
    {
        // cleanup failed
        perror("Stop server failed");
        exit(EXIT_FAILURE);
    }
}

bool HttpServer::CheckForNewClients(std::unique_ptr<HttpServerControl> &server, std::unique_ptr<HttpClient> &client)
{
    // Defining 1ms of waiting for timeout
    struct timeval timeout;
    struct fd_set fds;
    timeout.tv_sec = 0;
    timeout.tv_usec = 1000;
    FD_ZERO(&fds);
    FD_SET(socket_server, &fds);

    // Possible return values:
    // -1: error occurred
    // 0: timed out
    // > 0: data ready to be read
    int result = select(0, &fds, 0, 0, &timeout);
    if (result == -1)
    {
        // some error happened:
        printf("Server: Some error encountered with code number: %ld\n", WSAGetLastError());
    }
    else if (result > 0)
    {
        sockaddr_in client_addr;
        int iClient_addrLen;
        SOCKET socket_client;
        iClient_addrLen = sizeof(client_addr);
        socket_client = accept(socket_server, (sockaddr*)&client_addr, &iClient_addrLen);
        if (socket_client == INVALID_SOCKET)
        {
            // error handling code
            printf("Error accepting client: %ld\n", WSAGetLastError());
        }
        else
        {
            // Change the socket mode to non-block,
            // so it doesn't hang the server socket
            ULONG NonBlock = 1;
            if (ioctlsocket(socket_client, FIONBIO, &NonBlock) == SOCKET_ERROR)
            {
                perror("Not able to put the client socket in non-blocking mode");
                exit(EXIT_FAILURE);
            }

            auto client_ip = std::string(inet_ntoa(client_addr.sin_addr));
            auto client_port = ntohs(client_addr.sin_port);
            std::cout << "(" << socket_client << ") Client connected: " << client_ip << ":" << client_port << "\n";
            client.reset(new HttpClient(server, socket_client, client_ip, client_port));
            return true;
        }
    }

    return false;
}

#endif
