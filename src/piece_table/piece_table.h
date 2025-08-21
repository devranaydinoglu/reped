#pragma once

#include <string>
#include <vector>

#include "piece.h"

class PieceTable
{
private:
    std::string originalBuffer;
    std::string addBuffer;
    std::vector<Piece> pieces;
    std::size_t documentLength;

public:
    PieceTable();
    void readFile(const std::string& fileName);
    void readString(const std::string& str);
    void insert(std::string_view text, const std::size_t index);
    void remove(const std::size_t startIndex, const std::size_t endIndex);
    [[nodiscard]] std::string getText() const;
    [[nodiscard]] std::size_t getDocumentLength() const { return documentLength; }

private:
    std::tuple<std::size_t, Piece*> findPieceAtIndex(const std::size_t index);
};
