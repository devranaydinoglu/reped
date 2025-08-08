#include <iostream>
#include <memory>

#include "controller.h"
#include "../text_engine/text_engine.h"
#include "../text_engine/operations.h"
#include "../text_engine/input_events.h"
#include "../networking/client.h"

Controller::Controller()
    : textEngine(nullptr), client(nullptr)
{
}

int Controller::handleTextInputEvent(const TextInputEvent& event)
{    
    switch (event.type) {
        case TextInputEventType::INSERT:
        {
            std::cout << "Controller: INSERT operation - char '" << event.character << "' at position " << event.index << "\n";
            processLocalOperation(std::make_unique<InsertOperation>(event.character, event.index));
            break;
        }
        case TextInputEventType::DELETE:
        {
            std::cout << "Controller: DELETE operation at position " << event.index << "\n";
            processLocalOperation(std::make_unique<DeleteOperation>(event.index));
            break;
        }
        default: {
            return 1;
        }
    }
    
    return 0;
}

int Controller::handleCursorInputEvent(const CursorInputEvent& event)
{
    std::cout << "Controller: CURSOR_MOVE operation to position " << event.position << "\n";
    processLocalOperation(std::make_unique<CursorMoveOperation>(event.position));

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
            textEngine->insertCharLocal(insertOp);
            sendOperationToClient(*insertOp);
            break;
        }
        case OperationType::DELETE:
        {
            auto deleteOp = static_cast<DeleteOperation*>(operation.get());
            textEngine->deleteCharLocal(deleteOp);
            sendOperationToClient(*deleteOp);
            break;
        }
        case OperationType::CURSOR_MOVE:
        {
            auto cursorOp = static_cast<CursorMoveOperation*>(operation.get());
            textEngine->setCursorPosition(cursorOp->index);
            break;
        }
    }
}

void Controller::sendOperationToClient(const Operation& operation)
{
    if (!client) {
        std::cerr << "Controller: Client not set\n";
        return;
    }
    
    if (!client->isConnected())
    {
        std::cerr << "Controller: Client not connected\n";
        return;
    }

    std::string serialized = operation.serialize();
    if (client->sendMessage(serialized)) {
        std::cout << "Controller: Sent operation to client: " << serialized << "\n";
    } else {
        std::cerr << "Controller: Failed to send operation to client: " << serialized << "\n";
    }
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

void Controller::processIncomingMessage(const std::string &message)
{
    auto operation = Operation::deserialize(message);
    
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
            textEngine->insertCharIncoming(insertOp);
            break;
        }
        case OperationType::DELETE:
        {
            auto deleteOp = static_cast<DeleteOperation*>(operation.get());
            textEngine->deleteCharIncoming(deleteOp);
            break;
        }
        default:
            break;
    }
}
