#include <cstddef>

#include "piece.h"

Piece::Piece(BufferType bufferType, std::size_t start, std::size_t length)
    : bufferType(bufferType), start(start), length(length)
{
}
