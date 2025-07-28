#pragma once

#include <memory>

#include "./ui/window.h"
#include "./networking/server.h"
#include "./networking/client.h"

enum class AppMode
{
    NONE,
    CLIENT,
    SERVER
};

class Application
{
public:
    std::unique_ptr<Client> client;
    std::unique_ptr<Server> server;
    
    Window window;

public:
    Application();
    void onSetupCompleted(AppMode appMode, const uint16_t port, const std::string& serverAddress);
};