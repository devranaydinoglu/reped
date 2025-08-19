#pragma once

#include <vector>
#include <memory>

#include "text_engine.h"
#include "operations.h"

class ClientTextEngine : public TextEngine
{
private:
    std::vector<std::unique_ptr<TextOperation>> pendingLocalOps;
    std::vector<std::unique_ptr<TextOperation>> acknowledgedOps;

public:
    void addPendingLocalOp(std::unique_ptr<TextOperation> op);
    void acknowledgePendingOp(TextOperation* op);

    /**
    * Transforms operation against pending ops, applies op to local doc, retransforms pending ops against 
    * transformed op, and returns copy for broadcasting
    * @param op Op to process
    * @returns Copy of the transformed op
    */
    std::unique_ptr<TextOperation> processIncomingOperation(std::unique_ptr<TextOperation> op);
};
