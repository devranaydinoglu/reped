#include <memory>

#include "application.h"
#include "./ui/window.h"
#include "./text_engine/client_text_engine.h"
#include "./text_engine/server_text_engine.h"
#include "./controller/controller.h"

Application::Application()
    : client(nullptr), server(nullptr), controller(std::make_unique<Controller>()),
        textEngine(nullptr), clientId("")
{    
    // Setup window callbacks
    window.setOnSetupCompletedCallback([this] (AppMode appMode, const uint16_t port, const std::string& serverAddress, const std::string& clientId, const std::string& filePathName)
    {
        this->onSetupCompleted(appMode, port, serverAddress, clientId, filePathName);
    });
    
    window.setEditorController(controller.get());

    window.render();
}

void Application::onSetupCompleted(AppMode appMode, const uint16_t port, const std::string& serverAddress, const std::string& clientId, const std::string& filePathName)
{
    this->appMode = appMode;
    this->clientId = clientId;

    switch (appMode)
    {
        case AppMode::CLIENT:
            client = std::make_unique<Client>(port, serverAddress, controller.get(), clientId);
            controller->client = client.get();
            textEngine = std::make_unique<ClientTextEngine>();
            break;
        case AppMode::SERVER:
            server = std::make_unique<Server>(port, serverAddress, controller.get());
            textEngine = std::make_unique<ServerTextEngine>();

            if(filePathName.size() > 0)
                textEngine->readFile(filePathName);
            
            break;
        default:
            return;
    }

    controller->textEngine = textEngine.get();

    window.onSetupCompleted();
}
