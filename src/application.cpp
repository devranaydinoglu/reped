#include <memory>

#include "application.h"
#include "./ui/window.h"

Application::Application()
    : client(nullptr), server(nullptr)
{
    window.setOnAppModeSelectedCallback([this] (AppMode appMode, const uint16_t port, const std::string& serverAddress)
    {
        this->onSetupCompleted(appMode, port, serverAddress);
    });
    
    window.render();
}

void Application::onSetupCompleted(AppMode appMode, const uint16_t port, const std::string& serverAddress)
{
    switch (appMode)
    {
    case AppMode::CLIENT:
        client = std::move(std::make_unique<Client>(port, serverAddress));
        break;
    case AppMode::SERVER:
        server = std::make_unique<Server>(port);
        break;
    default:
        break;
    }
}
