#pragma once

#include <stdint.h>
#include <string>
#include <thread>
#include <atomic>
#include <functional>

class Controller;

class Client
{
public:
    std::string clientId;

private:
    const uint16_t port;
    const std::string serverAddress;
    int socketFd;
    std::atomic<bool> running;
    std::thread receiveThread;
    Controller* controller;

public:
    Client(const uint16_t port, const std::string& serverAddress, Controller* controller, const std::string& clientId);
    ~Client();

    [[nodiscard]] bool isConnected() const;

    /**
     * Sends a message to the server.
     * @returns True if the entire message was sent successfully.
    */
    [[nodiscard]] bool sendMessage(const std::string& message);

private:
    void connect();
    void disconnect();

    /**
     * Continuously receives messages from the server and invokes the message callback for each received message.
     * Runs in a separate thread when the client connects.
    */
    void receiveMessages();
};
