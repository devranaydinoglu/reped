#pragma once

#include <cstddef>
#include <string>
#include <iostream>

enum class OperationType
{
    INSERT,
    DELETE,
    CURSOR_MOVE
};

class Operation
{
public:
    OperationType type;
    std::size_t pos;

public:
    virtual ~Operation() = default;
    virtual std::string serialize() const = 0;
    static std::unique_ptr<Operation> deserialize(const std::string& message);
};

class TextOperation : public Operation
{
public:
    std::size_t length;
    std::string clientId;
    
    TextOperation(std::string clientId)
        : clientId(clientId) {}
};

class InsertOperation : public TextOperation
{
public:
    std::string text;

public:
    InsertOperation(std::string text, std::size_t pos, std::string clientId)
        : TextOperation(clientId), text(text)
    {
        this->pos = pos;
        type = OperationType::INSERT;
    }
    
    std::string serialize() const override
    {
        std::cout << "INSERT:" << clientId << ":" << pos << ":" << text << "\n";
        return "INSERT:" + clientId + ":" + std::to_string(pos) + ":" + text;
    }
};

class DeleteOperation : public TextOperation
{
public:
    DeleteOperation(std::size_t pos, std::size_t length, std::string clientId)
        : TextOperation(clientId)
    {
        this->pos = pos;
        this->length = length;
        type = OperationType::DELETE;
    }
    
    std::string serialize() const override
    {
        std::cout << "DELETE:" << clientId << ":" << pos << ":" << length << "\n";
        return "DELETE:" + clientId + ":" + std::to_string(pos) + ":" + std::to_string(length);
    }
};

class CursorMoveOperation : public Operation
{
public:
    CursorMoveOperation(std::size_t pos)
    {
        this->pos = pos;
        type = OperationType::CURSOR_MOVE;
    }
    
    std::string serialize() const override
    {
        std::cout << "CURSOR:" << pos << "\n";
        return "CURSOR:" + std::to_string(pos);
    }
};
