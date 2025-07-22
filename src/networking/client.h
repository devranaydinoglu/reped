#pragma once
#include <stdint.h>
#include <string>
#include <thread>
#include <atomic>
#include <functional>

class Client {
private:
    const uint16_t PORT = 8080;
    int socketFd = 0;
    std::thread receiveThread;
    std::atomic<bool> running;
    std::function<void(const std::string&)> onMessageReceived;

public:
    Client(const std::string& serverAddress = "localhost");
    ~Client();

    [[nodiscard]] bool isConnected() const;

    /**
     * Sends a message to the server.
     * @returns True if the entire message was sent successfully.
    */
    [[nodiscard]] bool sendMessage(const std::string& message);

    /**
     * Sets a callback function that will be called whenever a message is received from the server.
    */
    void setMessageReceivedCallback(std::function<void(const std::string&)> callback);

private:
    void connect(const std::string& serverAddress);
    void disconnect();

    /**
     * Continuously receives messages from the server and invokes the message callback for each received message.
     * Runs in a separate thread when the client connects.
    */
    void receiveMessages();
};
