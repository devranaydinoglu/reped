#pragma once

#include <stdint.h>
#include <vector>
#include <string>
#include <thread>
#include <mutex>
#include <atomic>
#include <unordered_map>

class Controller;
struct ParsedMessage;

class Server {
public:
    Controller* controller;

private:
    const uint16_t port;
    const std::string bindAddress;
    int socketFd;
    std::vector<int> clientSockets;
    std::unordered_map<int, std::string> clientIdMap;
    std::mutex clientsMutex;
    std::atomic<bool> running;
    std::thread acceptThread;

public:
    Server(const uint16_t port, const std::string& bindAddress, Controller* controller);
    ~Server();

private:
    void start();
    void stop();

    /**
     * Continuously accepts new client connections and adds their sockets to the clientSockets list.
     * For each new client, it spawns a thread to handle communication with that client.
    */
    void acceptClients();

    /**
     * Handles communication with a single client. Receives messages from the client and broadcasts them to all other clients.
     * Removes the client from the list and closes the socket when the client disconnects.
    */
    void handleClient(int clientSocket);

    /**
     * Broadcast incoming message to all connected clients except excludeSocket.
     * @param message Incoming message.
     * @param excludeSocket Socket to exclude from broadcast (client sending the message).
    */
    void broadcastToClients(const std::string& message, int excludeSocket = -1);

    void handleParsedMessage(const ParsedMessage& parsedMsg, int clientSocket);
};
