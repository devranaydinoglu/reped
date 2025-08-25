#include "text_engine.h"
#include "operations.h"
#include <memory>
#include <algorithm>

void TextEngine::insertLocal(InsertOperation* insertOp)
{
    std::cout << "TextEngine: INSERT local at position - " << insertOp->pos << " - text - " << insertOp->text << "\n";

    insertOp->docVersion = docVersion++;

    textBuffer.insert(insertOp->text, insertOp->pos);
    cursorPosition = insertOp->pos + insertOp->text.size();
}

void TextEngine::insertIncoming(InsertOperation* insertOp)
{
    std::cout << "TextEngine: INSERT incoming at position - " << insertOp->pos << " - text - " << insertOp->text << "\n";

    uint64_t oldVersion = docVersion;
    docVersion = std::max(docVersion, insertOp->docVersion) + 1;

    textBuffer.insert(insertOp->text, insertOp->pos);
}

void TextEngine::deleteLocal(DeleteOperation* deleteOp)
{
    std::cout << "TextEngine: DELETE local from position - " << deleteOp->pos << " - to position - " << deleteOp->pos + deleteOp->length << "\n";

    deleteOp->docVersion = docVersion++;
    
    if (deleteOp->length > 0 && deleteOp->pos >= 0 && deleteOp->pos + deleteOp->length <= textBuffer.getText().length())
    {
        textBuffer.remove(deleteOp->pos, deleteOp->pos + deleteOp->length);
        cursorPosition = deleteOp->pos;
    }
    else
    {
        std::cout << "TextEngine: Local delete operation out of bounds - skipping\n";
    }
}

void TextEngine::deleteIncoming(DeleteOperation* deleteOp)
{
    std::cout << "TextEngine: DELETE incoming from position - " << deleteOp->pos << " - to position - " << deleteOp->pos + deleteOp->length << "\n";

    docVersion = std::max(docVersion, deleteOp->docVersion) + 1;
    
    if (deleteOp->length > 0 && deleteOp->pos >= 0 && deleteOp->pos + deleteOp->length <= textBuffer.getText().length())
        textBuffer.remove(deleteOp->pos, deleteOp->pos + deleteOp->length);
    else
        std::cout << "TextEngine: Delete operation out of bounds - skipping\n";
}

void TextEngine::setCursorPosition(std::size_t pos)
{
    std::cout << "TextEngine: CURSOR_MOVE operation to position " << pos << "\n";
    cursorPosition = pos;
}

std::size_t TextEngine::getCursorPosition() const
{
    return cursorPosition;
}

std::string TextEngine::getText() const
{
    return textBuffer.getText();
}

std::size_t TextEngine::getDocumentLength() const
{
    return textBuffer.getDocumentLength();
}

void TextEngine::readFile(std::string filePathName)
{
    textBuffer.readFile(filePathName);
    docVersion = 0;
    cursorPosition = 0;
}

void TextEngine::readString(const std::string& str)
{
    textBuffer.readString(str);
    docVersion = 0;
    cursorPosition = 0;
}

std::unique_ptr<TextOperation> TextEngine::transform(const TextOperation* op1, const TextOperation* op2)
{    
    if (op1->type == OperationType::INSERT && op2->type == OperationType::INSERT)
    {
        auto insert1 = static_cast<const InsertOperation*>(op1);
        auto insert2 = static_cast<const InsertOperation*>(op2);
        
        auto transformed = std::make_unique<InsertOperation>(insert1->text, insert1->pos, insert1->clientId);
        transformed->operationId = insert1->operationId;
        transformed->docVersion = insert1->docVersion;
        
        if (insert2->pos < insert1->pos)
        {
            // op2 was inserted before or at op1's position, shift op1 right
            transformed->pos = insert1->pos + insert2->text.length();
        }
        else if (insert2->pos == insert1->pos)
        {
            // Tie-breaking: use clientId for deterministic ordering
            if (insert2->clientId < insert1->clientId)
                transformed->pos = insert1->pos + insert2->text.length();
            // else keep original position
        }
        
        return std::move(transformed);
    }
    else if (op1->type == OperationType::INSERT && op2->type == OperationType::DELETE)
    {
        auto insert1 = static_cast<const InsertOperation*>(op1);
        auto delete2 = static_cast<const DeleteOperation*>(op2);
        
        auto transformed = std::make_unique<InsertOperation>(insert1->text, insert1->pos, insert1->clientId);
        transformed->operationId = insert1->operationId;
        transformed->docVersion = insert1->docVersion;
        
        if (delete2->pos + delete2->length <= insert1->pos)
            // Delete range is completely before insert position
            transformed->pos = insert1->pos - delete2->length;
        else if (delete2->pos < insert1->pos)
            // Delete range overlaps with insert position
            transformed->pos = delete2->pos;
        // else delete is after insert, no change needed
        
        return std::move(transformed);
    }
    else if (op1->type == OperationType::DELETE && op2->type == OperationType::INSERT)
    {
        auto delete1 = static_cast<const DeleteOperation*>(op1);
        auto insert2 = static_cast<const InsertOperation*>(op2);
        
        auto transformed = std::make_unique<DeleteOperation>(delete1->pos, delete1->length, delete1->clientId);
        transformed->operationId = delete1->operationId;
        transformed->docVersion = delete1->docVersion;
        
        if (insert2->pos <= delete1->pos)
            // Insert is before delete range, shift delete right
            transformed->pos = delete1->pos + insert2->text.length();
        // else insert is after delete start, no change needed
        
        return std::move(transformed);
    }
    else if (op1->type == OperationType::DELETE && op2->type == OperationType::DELETE)
    {
        auto delete1 = static_cast<const DeleteOperation*>(op1);
        auto delete2 = static_cast<const DeleteOperation*>(op2);
        
        auto transformed = std::make_unique<DeleteOperation>(delete1->pos, delete1->length, delete1->clientId);
        transformed->operationId = delete1->operationId;
        transformed->docVersion = delete1->docVersion;
        
        if (delete2->pos + delete2->length <= delete1->pos)
        {
            // delete2 range is completely before delete1
            transformed->pos = delete1->pos - delete2->length;
        }
        else if (delete2->pos < delete1->pos + delete1->length && 
                   delete2->pos + delete2->length > delete1->pos)
        {
            // Ranges overlap - complex case
            if (delete2->pos <= delete1->pos)
            {
                // delete2 starts before or at delete1
                std::size_t overlap = std::min(delete2->pos + delete2->length - delete1->pos, delete1->length);
                transformed->pos = delete2->pos;
                transformed->length = delete1->length - overlap;
                
                if (transformed->length == 0)
                    // delete1 is completely contained in delete2, operation becomes no-op
                    return nullptr;
            }
            else
            {
                // delete2 starts within delete1 range
                transformed->length = delete2->pos - delete1->pos;
            }
        }
        // else delete2 is completely after delete1, no change needed
        
        return std::move(transformed);
    }
    
    auto transformed = std::make_unique<InsertOperation>("", op1->pos, op1->clientId);
    transformed->operationId = op1->operationId;
    transformed->type = op1->type;
    return std::move(transformed);
}
