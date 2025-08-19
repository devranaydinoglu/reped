#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <memory>
#include <sstream>

#include "server.h"
#include "../text_engine/operations.h"
#include "../controller/controller.h"

Server::Server(const uint16_t port, const std::string& bindAddress, Controller* controller)
    : port(port), bindAddress(bindAddress), socketFd(0), running(false), controller(controller)
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

    if (int status = getaddrinfo(bindAddress.c_str(), std::to_string(port).c_str(), &hints, &serverInfo) != 0)
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
    std::cout << "Server started on port " << port << " at address " << bindAddress << "\n";
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
    clientIdMap.clear();
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
        std::string msg(buffer);
        
        ParsedMessage parsedMsg = parseMessage(msg);
        
        std::string displayClientId = parsedMsg.clientId;
        if (displayClientId == "UNKNOWN")
        {
            // Fallback to stored mapping if not found in message
            std::lock_guard<std::mutex> lock(clientsMutex);
            auto it = clientIdMap.find(clientSocket);
            if (it != clientIdMap.end())
                displayClientId = it->second;
        }
        
        std::cout << "Received from Client " << clientSocket << " (ID: " << displayClientId << "): " << msg << "\n";

        handleParsedMessage(parsedMsg, clientSocket);
    }

    {
        std::lock_guard<std::mutex> lock(clientsMutex);
        auto it = std::find(clientSockets.begin(), clientSockets.end(), clientSocket);
        if (it != clientSockets.end())
            clientSockets.erase(it);
        
        clientIdMap.erase(clientSocket);
    }
    
    close(clientSocket);
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

ParsedMessage Server::parseMessage(const std::string& message)
{
    ParsedMessage parsed;

    std::istringstream ss(message);
    std::string token;

    std::vector<std::string> parts;
    while (std::getline(ss, token, ':'))
        parts.push_back(token);

    if (message.size() >= 10 && message.substr(0, 10) == "CONNECTED:")
    {
        parsed.type = MessageType::CONNECTED;
        parsed.clientId = message.substr(10);
        parsed.content = message;
        return parsed;
    }
    
    if (parts.size() >= 2)
    {
        if (parts[0] == "INSERT" || parts[0] == "DELETE")
        {
            parsed.type = MessageType::OPERATION;
            parsed.clientId = parts[1];
            parsed.content = message;
            return parsed;
        }
    }
    
    // Default to unknown message type
    parsed.type = MessageType::UNKNOWN;
    parsed.clientId = "UNKNOWN";
    parsed.content = message;
    return parsed;
}

void Server::handleParsedMessage(const ParsedMessage& parsedMsg, int clientSocket)
{
    switch (parsedMsg.type)
    {
        case MessageType::CONNECTED:
        {
            {
                std::lock_guard<std::mutex> lock(clientsMutex);
                clientIdMap[clientSocket] = parsedMsg.clientId;
            }
            std::cout << "Client " << clientSocket << " connected with ID: " << parsedMsg.clientId << "\n";
            break;
        }
            
        case MessageType::OPERATION:
        {
            std::unique_ptr<Operation> operation = Operation::deserialize(parsedMsg.content);
            if (!operation)
            {
                std::cerr << "Failed to deserialize operation: " << parsedMsg.content << " from client: " << parsedMsg.clientId << "\n";
                return;
            }

            // Apply to authoritative document through controller
            std::unique_ptr<Operation> transformedOp = controller->processIncomingMessage(parsedMsg.content);
            if (transformedOp) {
                std::string opMsg = transformedOp->serialize();
                
                broadcastToClients(opMsg, -1);
                std::cout << "Server: Broadcasted transformed operation: " << opMsg << "\n";
            }
            break;
        }
        
        default:
            std::cerr << "Unknown message type received from client " << clientSocket << "\n";
            break;
    }
}
