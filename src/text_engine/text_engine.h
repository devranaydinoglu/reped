#pragma once

#include <cstddef>
#include <string>

#include "../piece_table/piece_table.h"

class InsertOperation;
class DeleteOperation;

class TextEngine
{
private:
    PieceTable textBuffer;
    std::size_t cursorPosition;

public:
    TextEngine();
    void insertLocal(InsertOperation* insertOp);
    void insertIncoming(InsertOperation* insertOp);
    void deleteLocal(DeleteOperation* deleteOp);
    void deleteIncoming(DeleteOperation* deleteOp);
    void setCursorPosition(std::size_t position);
    std::size_t getCursorPosition() const { return cursorPosition; }
    std::string getText() const;

};
