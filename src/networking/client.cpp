#include "client.h"
#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>

Client::Client(const std::string& serverAddress)
    : running(false)
{
    connect(serverAddress);
}

Client::~Client()
{
    disconnect();
}

void Client::connect(const std::string& serverAddress)
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    struct addrinfo* serverInfo;

    if (int status = getaddrinfo(serverAddress.c_str(), std::to_string(PORT).c_str(), &hints, &serverInfo) != 0)
    {
        std::cerr << "Failed to get server info: " << gai_strerror(status) << "\n";
        return;
    }
    
    for (struct addrinfo* info = serverInfo; info != nullptr; info = info->ai_next)
    {
        socketFd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        
        if (socketFd == -1)
            continue;

        if (::connect(socketFd, info->ai_addr, info->ai_addrlen) != -1)
        {
            running = true;
            receiveThread = std::thread(&Client::receiveMessages, this);
            break;
        }

        close(socketFd);
    }

    freeaddrinfo(serverInfo);
}

void Client::disconnect()
{
    if (running)
    {
        running = false;
        close(socketFd);
        
        if (receiveThread.joinable())
            receiveThread.join();
    }
}

bool Client::isConnected() const
{
    return running;
}

bool Client::sendMessage(const std::string& message)
{
    if (!running)
        return false;
    
    ssize_t bytesSent = send(socketFd, message.c_str(), message.length(), 0);
    return bytesSent == static_cast<ssize_t>(message.length());
}

void Client::setMessageReceivedCallback(std::function<void(const std::string&)> callback)
{
    onMessageReceived = std::move(callback);
}

void Client::receiveMessages()
{
    char buffer[4096];
    
    while (running)
    {
        ssize_t bytesReceived = recv(socketFd, buffer, sizeof(buffer) - 1, 0);
        
        if (bytesReceived <= 0)
        {
            running = false;
            break;
        }

        buffer[bytesReceived] = '\0';
        std::string message(buffer);
        
        if (onMessageReceived)
            onMessageReceived(message);
    }
}
