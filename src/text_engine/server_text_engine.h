#pragma once

#include <vector>
#include <memory>

#include "text_engine.h"
#include "operations.h"

class TextOperation;

class ServerTextEngine : public TextEngine
{
private:
    std::vector<std::unique_ptr<TextOperation>> opHistory;

public:
    /**
    * Transforms operation against history, applies op to auth. doc, stores in opHistory,
    * and returns copy for broadcasting
    * @param op Op to process
    * @returns Copy of the transformed op
    */
    std::unique_ptr<TextOperation> processIncomingOperation(std::unique_ptr<TextOperation> op);
};
