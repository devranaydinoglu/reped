#pragma once

#include <cstddef>
#include <string>
#include <iostream>
#include <chrono>
#include <atomic>

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
    std::string operationId;
    std::size_t length;
    std::string clientId;
    uint64_t docVersion;
    
    TextOperation(std::string clientId)
        : clientId(clientId), docVersion(0)
    {
        operationId = generateOperationId(clientId);
    }
    
    TextOperation(const TextOperation& other)
        : clientId(other.clientId), operationId(other.operationId), 
          length(other.length), docVersion(other.docVersion)
    {
        this->pos = other.pos;
        this->type = other.type;
    }
    
private:
    static std::string generateOperationId(const std::string& clientId)
    {
        static std::atomic<uint64_t> sequence = 0;
        auto timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(
            std::chrono::system_clock::now().time_since_epoch()
        ).count();
        
        return clientId + "_" + std::to_string(timestamp) + "_" + std::to_string(sequence++);
    }
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
    
    InsertOperation(const InsertOperation& other)
        : TextOperation(other), text(other.text)
    {}
    
    std::string serialize() const override
    {
        return "INSERT:" + clientId + ":" + operationId + ":" + std::to_string(docVersion) + ":" + std::to_string(pos) + ":" + text;
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
    
    DeleteOperation(const DeleteOperation& other)
        : TextOperation(other)
    {}
    
    std::string serialize() const override
    {
        return "DELETE:" + clientId + ":" + operationId + ":" + std::to_string(docVersion) + ":" + std::to_string(pos) + ":" + std::to_string(length);
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
        return "CURSOR:" + std::to_string(pos);
    }
};
