#include <iostream>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <sstream>
#include <vector>

#include "client.h"
#include "../controller/controller.h"
#include "../text_engine/operations.h"
#include "../text_engine/client_text_engine.h"
#include "message_parser.h"

Client::Client(const uint16_t port, const std::string& serverAddress, Controller* controller, const std::string& clientId)
    : port(port), serverAddress(serverAddress), socketFd(0), running(false), controller(controller), clientId(clientId)
{
    connect();
}

Client::~Client()
{
    disconnect();
}

void Client::connect()
{
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo* serverInfo;

    if (int status = getaddrinfo(serverAddress.c_str(), std::to_string(port).c_str(), &hints, &serverInfo) != 0)
    {
        std::cerr << "Failed to get server info: " << gai_strerror(status) << "\n";
        return;
    }
    
    bool connected = false;
    for (struct addrinfo* info = serverInfo; info != nullptr; info = info->ai_next)
    {
        socketFd = socket(info->ai_family, info->ai_socktype, info->ai_protocol);
        
        if (socketFd == -1)
            continue;

        if (::connect(socketFd, info->ai_addr, info->ai_addrlen) != -1)
        {
            running = true;
            receiveThread = std::thread(&Client::receiveMessages, this);
            std::cout << "Client started on port " << port << " and connected to server at " << serverAddress << "\n";
            connected = true;
            
            std::string connectedMsg = MessageParser::createConnectedMessage(clientId);
            if (sendMessage(connectedMsg))
                std::cout << "Client: Sent operation to server: " << connectedMsg << "\n";
            else
                std::cerr << "Client: Failed to send operation to server: " << connectedMsg << "\n";

            break;
        }

        close(socketFd);
    }

    if (!connected)
        std::cerr << "Failed to connect to server at " << serverAddress << ":" << port << " - no server listening\n";
    
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
        std::string msg(buffer);
        
        std::cout << "Received: " << msg << "\n";

        ParsedMessage parsedMsg = MessageParser::parseMessage(msg);
        handleParsedMessage(parsedMsg);
    }
}

void Client::handleParsedMessage(const ParsedMessage& parsedMsg)
{
    if (parsedMsg.type == MessageType::INIT_DOCUMENT)
    {
        std::string initialContent = parsedMsg.content.substr(14);
        controller->setInitialDocument(initialContent);
    }
    else if (isAckMessage(parsedMsg.content))
    {
        handleAckMessage(parsedMsg.content);            
    }
    else
    {
        controller->processIncomingMessage(parsedMsg.content);
    }
}

bool Client::isAckMessage(const std::string& message) const
{
    std::istringstream ss(message);
    std::string token;
    std::vector<std::string> parts;
    
    while (std::getline(ss, token, ':')) {
        parts.push_back(token);
    }
    
    if (parts.size() >= 2) {
        std::string messageClientId = parts[1];
        return messageClientId == this->clientId;
    }
    
    return false;
}

void Client::handleAckMessage(const std::string& message)
{
    std::cout << "Received ACK: " << message << "\n";
    
    auto operation = Operation::deserialize(message);
    if (!operation) {
        std::cerr << "Failed to deserialize ACK message: " << message << "\n";
        return;
    }
    
    if (operation->type == OperationType::INSERT || operation->type == OperationType::DELETE)
    {
        auto textOp = static_cast<TextOperation*>(operation.get());
        
        ClientTextEngine* clientEngine = dynamic_cast<ClientTextEngine*>(controller->textEngine);
        if (clientEngine)
            clientEngine->acknowledgePendingOp(textOp);
    }
}
