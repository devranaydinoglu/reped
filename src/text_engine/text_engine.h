#pragma once

#include <cstddef>
#include <string>

#include "../piece_table/piece_table.h"
#include "operations.h"

class TextEngine
{
protected:
    PieceTable textBuffer;
    std::size_t cursorPosition;
    uint64_t docVersion;

public:
    TextEngine()
        : cursorPosition(0), docVersion(0)
    {}
    
    virtual ~TextEngine() = default;
    
    void insertLocal(InsertOperation* insertOp);
    void insertIncoming(InsertOperation* insertOp);
    void deleteLocal(DeleteOperation* deleteOp);
    void deleteIncoming(DeleteOperation* deleteOp);
    void setCursorPosition(std::size_t position);
    [[nodiscard]] std::size_t getCursorPosition() const;
    [[nodiscard]] std::string getText() const;
    [[nodiscard]] std::size_t getDocumentLength() const;
    void readFile(std::string filePathName);
    void readString(const std::string& str);

    /**
    * Transform op1 against op2.
    * @param op1 The op that will be transformed
    * @param op2 The op that op1 will be transformed against
    * @returns The transformed op
    */
    std::unique_ptr<TextOperation> transform(const TextOperation* op1, const TextOperation* op2);
};
