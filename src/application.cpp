#include <memory>

#include "application.h"
#include "./ui/window.h"
#include "./text_engine/text_engine.h"
#include "./controller/controller.h"

Application::Application()
    : client(nullptr), server(nullptr), controller(std::make_unique<Controller>(this)),
        textEngine(std::make_unique<TextEngine>()), clientId("")
{
    controller->textEngine = textEngine.get();
    
    // Setup window callbacks
    window.setOnSetupCompletedCallback([this] (AppMode appMode, const uint16_t port, const std::string& serverAddress, const std::string& clientId)
    {
        this->onSetupCompleted(appMode, port, serverAddress, clientId);
    });
    
    window.setEditorController(controller.get());

    window.render();
}

void Application::onSetupCompleted(AppMode appMode, const uint16_t port, const std::string& serverAddress, const std::string& clientId)
{
    this->appMode = appMode;
    this->clientId = clientId;

    switch (appMode)
    {
        case AppMode::CLIENT:
            client = std::make_unique<Client>(port, serverAddress, controller.get());
            controller->client = client.get();
            break;
        case AppMode::SERVER:
            server = std::make_unique<Server>(port, serverAddress, controller.get());
            break;
        default:
            return;
    }

    window.onSetupCompleted();
}
