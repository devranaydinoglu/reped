#include "client_text_engine.h"
#include <algorithm>
#include <iostream>

void ClientTextEngine::addPendingLocalOp(std::unique_ptr<TextOperation> op)
{
    pendingLocalOps.emplace_back(std::move(op));
}

void ClientTextEngine::acknowledgePendingOp(TextOperation *op)
{
    auto it = std::find_if(pendingLocalOps.begin(), pendingLocalOps.end(), [op](const std::unique_ptr<TextOperation>& pendingOp)
        {
            return pendingOp->operationId == op->operationId;
        });
    
    if (it != pendingLocalOps.end())
    {
        acknowledgedOps.emplace_back(std::move(*it));
        pendingLocalOps.erase(it);
        
        std::cout << "ClientTextEngine: Acknowledged operation " << op->operationId << "\n";
    }
    else
    {
        std::cerr << "ClientTextEngine: Could not find pending operation " << op->operationId << " to acknowledge\n";
    }
}

std::unique_ptr<TextOperation> ClientTextEngine::processIncomingOperation(std::unique_ptr<TextOperation> op) 
{
    // Transform incoming op against all pending local ops
    auto transformedOp = std::move(op);
    for (const auto& pendingOp : pendingLocalOps)
    {
        auto newTransformed = transform(transformedOp.get(), pendingOp.get());
        if (newTransformed)
            transformedOp = std::move(newTransformed);
    }

    // Apply transformed op to local doc
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

    // Retransform all pending local ops against the transformed incoming op
    for (auto& pendingOp : pendingLocalOps)
    {
        auto retransformed = transform(pendingOp.get(), transformedOp.get());
        if (retransformed)
        {
            pendingOp = std::move(retransformed);
        }
    }

    return transformedOp;
}