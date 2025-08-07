#pragma once

#include <memory>

#include "./ui/window.h"
#include "./networking/server.h"
#include "./networking/client.h"
#include "./text_engine/text_engine.h"
#include "./controller/controller.h"

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
    std::unique_ptr<Controller> controller;
    std::unique_ptr<TextEngine> textEngine;
    
    Window window;

public:
    Application();

private:
    void onSetupCompleted(AppMode appMode, const uint16_t port, const std::string& serverAddress);
};