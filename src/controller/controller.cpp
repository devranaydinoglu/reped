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
    std::unique_ptr<Operation> operation = nullptr;
    
    switch (event.type) {
        case TextInputEventType::INSERT: {
            operation = std::make_unique<InsertOperation>(event.character, event.index);
            std::cout << "Controller: INSERT operation - char '" << event.character << "' at position " << event.index << "\n";
            break;
        }
        case TextInputEventType::DELETE: {
            operation = std::make_unique<DeleteOperation>(event.index);
            std::cout << "Controller: DELETE operation at position " << event.index << "\n";
            break;
        }
    }
    
    if (operation) {
        processOperation(std::move(operation));
    }
    
    return 0;
}

int Controller::handleCursorInputEvent(const CursorInputEvent& event)
{
    std::unique_ptr<CursorMoveOperation> operation = std::make_unique<CursorMoveOperation>(event.position);
    std::cout << "Controller: CURSOR_MOVE operation to position " << event.position << "\n";
    
    processOperation(std::move(operation));
    return 0;
}

void Controller::processOperation(std::unique_ptr<Operation> operation)
{
    if (!textEngine) {
        std::cerr << "Controller: TextEngine not set!\n";
        return;
    }
    
    switch (operation->type) {
        case OperationType::INSERT: {
            auto insertOp = static_cast<InsertOperation*>(operation.get());
            textEngine->insertChar(insertOp);
            break;
        }
        case OperationType::DELETE: {
            auto deleteOp = static_cast<DeleteOperation*>(operation.get());
            textEngine->deleteChar(deleteOp);
            break;
        }
        case OperationType::CURSOR_MOVE: {
            auto cursorOp = static_cast<CursorMoveOperation*>(operation.get());
            textEngine->setCursorPosition(cursorOp->index);
            break;
        }
    }
    
    if (client && client->isConnected()) {
        sendOperationToClient(*operation);
    }
    
    if (onOperationProcessed) {
        onOperationProcessed(operation->serialize());
    }
}

void Controller::sendOperationToClient(const Operation& operation)
{
    if (!client) {
        std::cerr << "Controller: Client not set!\n";
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
