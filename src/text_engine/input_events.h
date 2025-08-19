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
    std::string text;
    std::size_t pos;
    std::size_t length; // For delete op

public:
    TextInputEvent(TextInputEventType eventType, std::string text, std::size_t pos, std::size_t length = 1)
        : type(eventType), text(text), pos(pos), length(length) {}
};

class CursorInputEvent
{
public:
    std::size_t pos;

public:
    CursorInputEvent(std::size_t pos)
        : pos(pos) {}    
}; 
