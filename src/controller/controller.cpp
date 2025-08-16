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
            std::cout << "Controller: INSERT operation - text '" << event.text << "' at position " << event.pos << "\n";
            processLocalOperation(std::make_unique<InsertOperation>(event.text, event.pos, "4"));
            break;
        }
        case TextInputEventType::DELETE:
        {
            std::cout << "Controller: DELETE operation at position " << event.pos << "\n";
            processLocalOperation(std::make_unique<DeleteOperation>(event.pos, event.length, "4"));
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
    std::cout << "Controller: CURSOR_MOVE operation to position " << event.pos << "\n";
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
            sendOperationToClient(*insertOp);
            break;
        }
        case OperationType::DELETE:
        {
            auto deleteOp = static_cast<DeleteOperation*>(operation.get());
            textEngine->deleteLocal(deleteOp);
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
            textEngine->insertIncoming(insertOp);
            break;
        }
        case OperationType::DELETE:
        {
            auto deleteOp = static_cast<DeleteOperation*>(operation.get());
            textEngine->deleteIncoming(deleteOp);
            break;
        }
        default:
            break;
    }
}
