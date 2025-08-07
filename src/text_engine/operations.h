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
    std::size_t index;

public:
    virtual ~Operation() = default;
    virtual std::string serialize() const = 0;
};

class InsertOperation : public Operation
{
public:
    char character;

public:
    InsertOperation(char c, std::size_t pos)
        : character(c)
    {
        index = pos;
        type = OperationType::INSERT;
    }
    
    std::string serialize() const override
    {
        std::cout << "INSERT:" << index << ":" << character << "\n";
        return "INSERT:" + std::to_string(index) + ":" + character;
    }
};

class DeleteOperation : public Operation
{
public:
    DeleteOperation(std::size_t pos)
    {
        index = pos;
        type = OperationType::DELETE;
    }
    
    std::string serialize() const override
    {
        std::cout << "DELETE:" << index << "\n";
        return "DELETE:" + std::to_string(index);
    }
};

class CursorMoveOperation : public Operation
{
public:
    CursorMoveOperation(std::size_t pos)
    {
        index = pos;
        type = OperationType::CURSOR_MOVE;
    }
    
    std::string serialize() const override
    {
        std::cout << "CURSOR:" << index << "\n";
        return "CURSOR:" + std::to_string(index);
    }
};
