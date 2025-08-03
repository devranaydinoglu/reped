#include <fstream>
#include <streambuf>
#include <iostream>
#include <cstddef>
#include <tuple>
#include <string>
#include <algorithm>

#include "piece_table.h"
#include "piece.h"

PieceTable::PieceTable()
    : documentLength(0)
{
}

void PieceTable::read(const std::string& fileName)
{
    std::ifstream fileStream(fileName);
    if (!fileStream)
    {
        std::cerr << "Failed to open file: " << fileName << "\n";
        return;
    }

    pieces.clear();
    originalBuffer.clear();
    addBuffer.clear();
    documentLength = 0;

    fileStream.seekg(0, std::ios::end);
    documentLength = fileStream.tellg();
    fileStream.seekg(0);

    if (documentLength > 0)
    {
        originalBuffer.resize(documentLength);
        fileStream.read(&originalBuffer[0], documentLength);
    }

    pieces.emplace_back(BufferType::ORIGINAL, 0, documentLength);

    std::cout << "Characters: " << documentLength << "\n";
}

void PieceTable::insert(std::string_view text, const std::size_t index)
{
    if (text.empty())
        return;
    
    if (index > documentLength)
    {
        insert(text, documentLength);
        return;
    }

    std::size_t textStartIndex = addBuffer.size();
    std::size_t textLength = text.size();
    documentLength += textLength;
    addBuffer.append(text);

    // Insert in empty document
    if (pieces.empty())
    {
        pieces.emplace_back(BufferType::ADD, textStartIndex, textLength);
        return;
    }

    // Insert at end of document
    if (index == documentLength - textLength)
    {
        pieces.emplace_back(BufferType::ADD, textStartIndex, textLength);
        return;
    }

    // Find piece containing the insertion index
    auto [pieceOffset, piecePtr] = findPieceAtIndex(index);
    
    if (!piecePtr)
    {
        pieces.emplace_back(BufferType::ADD, textStartIndex, textLength);
        return;
    }

    std::size_t offsetWithinPiece = index - pieceOffset;
    std::vector<Piece> newPieces;

    // Add piece before insertion index
    if (offsetWithinPiece > 0)
        newPieces.emplace_back(piecePtr->bufferType, piecePtr->start, offsetWithinPiece);

    // Add the inserted text piece
    newPieces.emplace_back(BufferType::ADD, textStartIndex, textLength);

    // Add piece after insertion index
    std::size_t remainingLength = piecePtr->length - offsetWithinPiece;
    if (remainingLength > 0)
        newPieces.emplace_back(piecePtr->bufferType, piecePtr->start + offsetWithinPiece, remainingLength);

    // Remove the original piece
    auto pieceIt = std::find(pieces.begin(), pieces.end(), *piecePtr);
    if (pieceIt != pieces.end())
    {
        auto insertPos = pieces.erase(pieceIt);
        pieces.insert(insertPos, newPieces.begin(), newPieces.end());
    }
}

void PieceTable::remove(const std::size_t startIndex, const std::size_t endIndex)
{
    if (startIndex >= documentLength || startIndex >= endIndex)
        return;

    std::size_t actualEndIndex = std::min(endIndex, documentLength);
    std::size_t removeLength = actualEndIndex - startIndex;

    if (removeLength == 0)
        return;

    // Find the pieces that contain the start and end of the range to remove
    auto [startPieceOffset, startPiecePtr] = findPieceAtIndex(startIndex);
    auto [endPieceOffset, endPiecePtr] = findPieceAtIndex(actualEndIndex - 1);

    if (!startPiecePtr)
        return;

    std::vector<Piece> newPieces;
    auto startIt = std::find(pieces.begin(), pieces.end(), *startPiecePtr);
    auto endIt = std::find(pieces.begin(), pieces.end(), *endPiecePtr);

    // Handle the piece containing the start of the range
    std::size_t startOffsetInPiece = startIndex - startPieceOffset;
    if (startOffsetInPiece > 0)
        newPieces.emplace_back(startPiecePtr->bufferType, startPiecePtr->start, startOffsetInPiece);

    // Handle the piece containing the end of the range
    if (startPiecePtr == endPiecePtr)
    {
        // Start and end are in the same piece
        std::size_t endOffsetInPiece = actualEndIndex - startPieceOffset;
        std::size_t remainingLength = startPiecePtr->length - endOffsetInPiece;
        if (remainingLength > 0)
            newPieces.emplace_back(startPiecePtr->bufferType, startPiecePtr->start + endOffsetInPiece, remainingLength);
    }
    else
    {
        // Start and end are in different pieces
        std::size_t endOffsetInPiece = actualEndIndex - endPieceOffset;
        std::size_t remainingLength = endPiecePtr->length - endOffsetInPiece;
        if (remainingLength > 0)
            newPieces.emplace_back(endPiecePtr->bufferType, endPiecePtr->start + endOffsetInPiece, remainingLength);
    }

    // Remove the affected pieces and insert the new ones
    auto eraseStart = pieces.erase(startIt, endIt + 1);
    pieces.insert(eraseStart, newPieces.begin(), newPieces.end());

    documentLength -= removeLength;
}

std::string PieceTable::getText() const
{
    std::string result;
    result.reserve(documentLength);

    for (const auto& piece : pieces) {
        const std::string& buffer = (piece.bufferType == BufferType::ORIGINAL) ? originalBuffer : addBuffer;
        result.append(buffer, piece.start, piece.length);
    }

    return result;
}

std::tuple<std::size_t, Piece*> PieceTable::findPieceAtIndex(const std::size_t index)
{
    std::size_t globalOffset = 0;

    for (auto& piece : pieces) {
        if (index < globalOffset + piece.length) {
            return {globalOffset, &piece};
        }
        globalOffset += piece.length;
    }

    if (!pieces.empty()) {
        std::size_t lastPieceOffset = globalOffset - pieces.back().length;
        return {lastPieceOffset, &pieces.back()};
    }

    return {0, nullptr};
}
