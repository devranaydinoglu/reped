#include <memory>

#include "application.h"
#include "./ui/window.h"
#include "./text_engine/text_engine.h"
#include "./controller/controller.h"

Application::Application()
    : client(nullptr), server(nullptr), textEngine(std::make_unique<TextEngine>()), 
        controller(std::make_unique<Controller>())
{
    controller->setTextEngine(textEngine.get());
    
    
    // Setup window callbacks
    window.setOnSetupCompletedCallback([this] (AppMode appMode, const uint16_t port, const std::string& serverAddress)
    {
        this->onSetupCompleted(appMode, port, serverAddress);
    });
    

    window.setEditorController(controller.get());

    window.render();
}

void Application::onSetupCompleted(AppMode appMode, const uint16_t port, const std::string& serverAddress)
{
    switch (appMode)
    {
        case AppMode::CLIENT:
            client = std::make_unique<Client>(port, serverAddress);
            controller->setClient(client.get());
            break;
        case AppMode::SERVER:
            server = std::make_unique<Server>(port, serverAddress);
            break;
        default:
            return;
    }

    window.onSetupCompleted();
}
