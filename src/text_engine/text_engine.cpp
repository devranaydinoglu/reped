#include "text_engine.h"
#include "operations.h"

TextEngine::TextEngine()
    : cursorPosition(0)
{
}

void TextEngine::insertChar(InsertOperation* insertOp)
{
    std::string charStr(1, insertOp->character);
    textBuffer.insert(charStr, insertOp->index);
    cursorPosition = insertOp->index + 1;
}

void TextEngine::deleteChar(DeleteOperation* deleteOp)
{
    if (deleteOp->index > 0) {
        textBuffer.remove(deleteOp->index - 1, deleteOp->index);
        cursorPosition = deleteOp->index - 1;
    }
}

void TextEngine::setCursorPosition(std::size_t position)
{
    cursorPosition = position;
}

std::string TextEngine::getText() const
{
    return textBuffer.getText();
}
