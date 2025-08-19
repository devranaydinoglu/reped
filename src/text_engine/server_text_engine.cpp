#include "server_text_engine.h"

std::unique_ptr<TextOperation> ServerTextEngine::processIncomingOperation(std::unique_ptr<TextOperation> op) 
{
    // Transform against opHistory based on document versions
    auto transformedOp = std::move(op);
    for (const auto& historyOp : opHistory)
    {
        // Only transform against operations that happened after the incoming operation was created
        if (historyOp->docVersion > transformedOp->docVersion) {
            auto newTransformed = transform(transformedOp.get(), historyOp.get());
            if (newTransformed)
                transformedOp = std::move(newTransformed);
        }
    }
    
    transformedOp->docVersion = docVersion;
    
    // Apply transformed op to authoritative document
    if (transformedOp->type == OperationType::INSERT)
    {
        auto insertOp = static_cast<InsertOperation*>(transformedOp.get());
        insertIncoming(insertOp);
    }
    else if (transformedOp->type == OperationType::DELETE)
    {
        auto deleteOp = static_cast<DeleteOperation*>(transformedOp.get());
        deleteIncoming(deleteOp);
    }
    
    // Create a copy for broadcasting
    std::unique_ptr<TextOperation> broadcastCopy;
    if (transformedOp->type == OperationType::INSERT)
    {
        auto insertOp = static_cast<InsertOperation*>(transformedOp.get());
        broadcastCopy = std::make_unique<InsertOperation>(*insertOp);
    }
    else if (transformedOp->type == OperationType::DELETE)
    {
        auto deleteOp = static_cast<DeleteOperation*>(transformedOp.get());
        broadcastCopy = std::make_unique<DeleteOperation>(*deleteOp);
    }
    
    opHistory.push_back(std::move(transformedOp));
    
    return broadcastCopy;
}
