#pragma once

#include <memory>
#include <functional>

class TextEngine;
class Client;
class Operation;
class TextInputEvent;
class CursorInputEvent;

class Controller
{
public:
    TextEngine* textEngine;
    Client* client;

public:
    Controller();
    
    int handleTextInputEvent(const TextInputEvent& event);
    int handleCursorInputEvent(const CursorInputEvent& event);
    std::string getText() const;
    std::size_t getCursorPosition() const;
    void setCursorPosition(std::size_t position);
    std::unique_ptr<Operation> processIncomingMessage(const std::string& message);

private:
    void processLocalOperation(std::unique_ptr<Operation> operation);
    void sendOperationToClient(const Operation& operation);
};