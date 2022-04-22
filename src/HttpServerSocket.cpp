#ifndef __WIN64
#ifndef ARDUINO

#include <iostream>

#include <memory>
#include <string>
#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>	//inet_addr
//#include <sys/ioctl.h>
#include <fcntl.h>
#include <errno.h>
#include "GlobalSettings.h"
#include "HttpClient.h"
#include "HttpServerSocket.h"

void HttpServer::Start()
{
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[1024] = {0};
       
    // Creating socket file descriptor
    if ((id_server_socket = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Adding the flag for non-blocking listening socket
    int flags = fcntl(id_server_socket, F_GETFL);
    if (fcntl(id_server_socket, F_SETFL, flags | O_NONBLOCK) < 0)
    {
        perror("setsocknonblocking");
        exit(EXIT_FAILURE);
    }
       
    // Forcefully attaching socket to the port
    if (setsockopt(id_server_socket, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt)))
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(HTTP_PORT);
       
    // Forcefully attaching socket to the port 8080
    if (bind(id_server_socket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(id_server_socket, MAX_HTTP_CLIENTS) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

void HttpServer::Stop()
{
    shutdown(id_server_socket, SHUT_RDWR);
}

bool HttpServer::CheckForNewClients(std::unique_ptr<HttpServerControl> &server, std::unique_ptr<HttpClient> &client)
{
    struct sockaddr_in client_addr; // client's address information
    socklen_t sin_size;
    sin_size = sizeof client_addr;

    int id_client_socket = accept(id_server_socket, (struct sockaddr *)&client_addr, &sin_size);

    if (id_client_socket == -1)
    {
        if (errno != EWOULDBLOCK)
        {
            perror("Error when accepting connection");
            exit(EXIT_FAILURE);
        }
        return false;
    }
    else
    {
        auto client_ip = std::string(inet_ntoa(client_addr.sin_addr));
        auto client_port = ntohs(client_addr.sin_port);
        std::cout << "(" << id_client_socket << ") Client connected: " << client_ip << ":" << client_port << "\n";
        client.reset(new HttpClient(server, id_client_socket, client_ip, client_port));
        return true;
    }
}

#endif

#endif
