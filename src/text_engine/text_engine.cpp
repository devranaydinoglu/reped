#include "text_engine.h"
#include "operations.h"

TextEngine::TextEngine()
    : cursorPosition(0)
{
}

void TextEngine::insertLocal(InsertOperation* insertOp)
{
    // Apply operation locally (optimistic update)
    textBuffer.insert(insertOp->text, insertOp->pos);
    cursorPosition = insertOp->pos + insertOp->text.size();
}

void TextEngine::insertIncoming(InsertOperation* insertOp)
{
    textBuffer.insert(insertOp->text, insertOp->pos);
}

void TextEngine::deleteLocal(DeleteOperation* deleteOp)
{
    if (deleteOp->length > 0 && deleteOp->pos >= 0 && deleteOp->pos < textBuffer.getText().length())
    {
        textBuffer.remove(deleteOp->pos, deleteOp->pos + deleteOp->length);
        cursorPosition = deleteOp->pos;
    }
}

void TextEngine::deleteIncoming(DeleteOperation* deleteOp)
{    
    if (deleteOp->length > 0 && deleteOp->pos >= 0 && deleteOp->pos < textBuffer.getText().length())
    {
        textBuffer.remove(deleteOp->pos, deleteOp->pos + deleteOp->length);
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
