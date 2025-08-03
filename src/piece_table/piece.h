#pragma once

enum class BufferType
{
    NONE,
    ORIGINAL,
    ADD
};

class Piece
{
public:
    BufferType bufferType;
    std::size_t start;
    std::size_t length;

public:
    Piece(BufferType bufferType, std::size_t start, std::size_t length);

    bool operator==(const Piece& other) const
    {
        return other.bufferType == bufferType &&
                other.start == start &&
                other.length == length;
    }
};