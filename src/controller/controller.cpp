#include <iostream>
#include <memory>

#include "controller.h"
#include "../text_engine/text_engine.h"
#include "../text_engine/client_text_engine.h"
#include "../text_engine/server_text_engine.h"
#include "../text_engine/operations.h"
#include "../text_engine/input_events.h"
#include "../networking/client.h"

Controller::Controller()
    : textEngine(nullptr), client(nullptr)
{
}

int Controller::handleTextInputEvent(const TextInputEvent& event)
{    
    switch (event.type)
    {
        case TextInputEventType::INSERT:
        {
            processLocalOperation(std::make_unique<InsertOperation>(event.text, event.pos, client->clientId));
            break;
        }
        case TextInputEventType::DELETE:
        {
            processLocalOperation(std::make_unique<DeleteOperation>(event.pos, event.length, client->clientId));
            break;
        }
        default:
            return 1;
    }
    
    return 0;
}

int Controller::handleCursorInputEvent(const CursorInputEvent& event)
{
    processLocalOperation(std::make_unique<CursorMoveOperation>(event.pos));

    return 0;
}

void Controller::processLocalOperation(std::unique_ptr<Operation> operation)
{
    if (!textEngine)
    {
        std::cerr << "Controller: TextEngine not set\n";
        return;
    }
    
    switch (operation->type)
    {
        case OperationType::INSERT:
        {
            auto insertOp = static_cast<InsertOperation*>(operation.get());
            textEngine->insertLocal(insertOp);
            
            ClientTextEngine* clientEngine = dynamic_cast<ClientTextEngine*>(textEngine);
            if (clientEngine)
            {
                auto pendingOp = std::make_unique<InsertOperation>(*insertOp);
                clientEngine->addPendingLocalOp(std::move(pendingOp));
            }
            
            sendOperationToClient(*insertOp);
            break;
        }
        case OperationType::DELETE:
        {
            auto deleteOp = static_cast<DeleteOperation*>(operation.get());
            textEngine->deleteLocal(deleteOp);
            
            ClientTextEngine* clientEngine = dynamic_cast<ClientTextEngine*>(textEngine);
            if (clientEngine)
            {
                auto pendingOp = std::make_unique<DeleteOperation>(*deleteOp);
                clientEngine->addPendingLocalOp(std::move(pendingOp));
            }
            
            sendOperationToClient(*deleteOp);
            break;
        }
        case OperationType::CURSOR_MOVE:
        {
            auto cursorOp = static_cast<CursorMoveOperation*>(operation.get());
            textEngine->setCursorPosition(cursorOp->pos);
            break;
        }
    }
}

void Controller::sendOperationToClient(const Operation& operation)
{
    if (!client)
    {
        std::cerr << "Controller: Client not set\n";
        return;
    }
    
    if (!client->isConnected())
    {
        std::cerr << "Controller: Client not connected\n";
        return;
    }

    std::string serialized = operation.serialize();
    if (client->sendMessage(serialized))
        std::cout << "Controller: Sent operation to client: " << serialized << "\n";
    else
        std::cerr << "Controller: Failed to send operation to client: " << serialized << "\n";
}

std::string Controller::getText() const
{
    return textEngine->getText();
}

std::size_t Controller::getCursorPosition() const
{
    return textEngine->getCursorPosition();
}

void Controller::setCursorPosition(std::size_t position)
{
    return textEngine->setCursorPosition(position);
}

void Controller::setInitialDocument(const std::string& str)
{
    textEngine->readString(str);
    std::cout << "Controller: Set initial document with " << str.length() << " characters\n";
}

std::unique_ptr<Operation> Controller::processIncomingMessage(const std::string &message)
{
    if (!textEngine)
    {
        std::cerr << "Controller: TextEngine not set\n";
        return nullptr;
    }

    std::unique_ptr<Operation> op = Operation::deserialize(message);
    auto textOp = std::unique_ptr<TextOperation>(static_cast<TextOperation*>(op.release()));

    ServerTextEngine* serverEngine = dynamic_cast<ServerTextEngine*>(textEngine);
    if (serverEngine)
        return serverEngine->processIncomingOperation(std::move(textOp));

    ClientTextEngine* clientEngine = dynamic_cast<ClientTextEngine*>(textEngine);
    if (clientEngine)
    {
        clientEngine->processIncomingOperation(std::move(textOp));
        return nullptr;
    }

    return nullptr;
}

std::string Controller::getClientId() const
{
    if (client)
        return client->getClientId();
    
    return "Server";
}
