#pragma once

#include <cstddef>

enum class TextInputEventType
{
    INSERT,
    DELETE
};

class TextInputEvent
{
public:
    TextInputEventType type;
    char character;
    std::size_t index;

public:
    TextInputEvent(TextInputEventType eventType, char c, std::size_t pos)
        : type(eventType), character(c), index(pos) {}
};

class CursorInputEvent
{
public:
    std::size_t position;

public:
    CursorInputEvent(std::size_t pos) : position(pos) {}    
}; 