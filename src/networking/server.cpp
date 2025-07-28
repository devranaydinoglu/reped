#include "server.h"
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

Server::Server(const uint16_t port)
    : port(port), socketFd(0), running(false)
{
    start();
}

Server::~Server()
{
    stop();
}

void Server::start()
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    struct addrinfo* serverInfo;
    struct addrinfo* info;

    if (int status = getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &serverInfo) != 0)
    {
        std::cerr << "Failed to get server info: " << gai_strerror(status) << std::endl;
        return;
    }

    for (info = serverInfo; info != nullptr; info = info->ai_next)
    {
        socketFd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        
        if (socketFd == -1)
            continue;

        int reuse = 1;
        if (setsockopt(socketFd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) == -1)
        {
            std::cerr << "Failed to set socket options\n";
            close(socketFd);
            continue;
        }

        if (bind(socketFd, info->ai_addr, info->ai_addrlen) == 0)
            break;

        close(socketFd);
    }
    
    freeaddrinfo(serverInfo);
    
    if (info == nullptr)
    {
        std::cerr << "Could not bind to any address\n";
        return;
    }

    if (listen(socketFd, 10) == -1)
    {
        std::cerr << "Failed to listen on socket\n";
        return;
    }

    running = true;
    acceptThread = std::thread(&Server::acceptClients, this);
    std::cout << "Server started on port " << port << "\n";
}

void Server::stop()
{
    if (!running)
        return;
    
    running = false;
    close(socketFd);
    
    if (acceptThread.joinable())
        acceptThread.join();

    std::lock_guard<std::mutex> lock(clientsMutex);
    for (int clientSocket : clientSockets)
        close(clientSocket);

    clientSockets.clear();
}

void Server::acceptClients()
{
    while (running)
    {
        struct sockaddr_storage clientAddr;
        socklen_t addrSize = sizeof(clientAddr);
        
        int clientSocket = accept(socketFd, (struct sockaddr*)&clientAddr, &addrSize);
        
        if (clientSocket == -1)
        {
            if (running)
                std::cerr << "Failed to accept client connection\n";
            
            continue;
        }

        {
            std::lock_guard<std::mutex> lock(clientsMutex);
            clientSockets.push_back(clientSocket);
        }

        std::thread(&Server::handleClient, this, clientSocket).detach();
        std::cout << "New client connected\n";
    }
}

void Server::handleClient(int clientSocket)
{
    char buffer[4096];
    
    while (running)
    {
        ssize_t bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesReceived <= 0)
            break;

        buffer[bytesReceived] = '\0';
        std::string message(buffer);
        
        broadcastToClients(message, clientSocket);
    }

    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        auto it = std::find(clientSockets.begin(), clientSockets.end(), clientSocket);
        if (it != clientSockets.end())
            clientSockets.erase(it);
    }
    
    close(clientSocket);
    std::cout << "Client disconnected\n";
}

void Server::broadcastToClients(const std::string& message, int excludeSocket)
{
    std::lock_guard<std::mutex> lock(clientsMutex);
    
    for (int clientSocket : clientSockets)
    {
        if (clientSocket != excludeSocket)
            send(clientSocket, message.c_str(), message.length(), 0);
    }
}
