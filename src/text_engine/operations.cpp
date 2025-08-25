#include "operations.h"

#include <memory>
#include <sstream>
#include <vector>

#include "operations.h"

#include <memory>
#include <sstream>
#include <vector>

std::unique_ptr<Operation> Operation::deserialize(const std::string& str)
{
    size_t firstColon = str.find(':');
    if (firstColon == std::string::npos)
        return nullptr;
        
    std::string opType = str.substr(0, firstColon);
    
    if (opType == "INSERT")
    {
        std::istringstream ss(str);
        std::string token;
        std::vector<std::string> parts;
        
        int partCount = 0;
        size_t lastPos = 0;
        
        while (partCount < 5 && std::getline(ss, token, ':'))
        {
            parts.push_back(token);
            lastPos = ss.tellg();
            partCount++;
        }
        
        if (partCount < 5 || lastPos == -1) // INSERT:clientId:operationId:docVersion:pos:text
            return nullptr;
            
        // Get the text portion (everything after the 5th colon)
        std::string text = str.substr(lastPos);
        
        std::string clientId = parts[1];
        std::string operationId = parts[2];
        uint64_t docVersion = std::stoull(parts[3]);
        std::size_t pos = std::stoul(parts[4]);
        
        auto op = std::make_unique<InsertOperation>(text, pos, clientId);
        op->operationId = operationId;
        op->docVersion = docVersion;
        return std::move(op);
    }
    else if (opType == "DELETE")
    {
        std::istringstream ss(str);
        std::string token;

        std::vector<std::string> parts;
        while (std::getline(ss, token, ':'))
            parts.push_back(token);
            
        if (parts.size() < 6)  // DELETE:clientId:operationId:docVersion:pos:length
            return nullptr;
        
        std::string clientId = parts[1];
        std::string operationId = parts[2];
        uint64_t docVersion = std::stoull(parts[3]);
        std::size_t pos = std::stoul(parts[4]);
        std::size_t length = std::stoul(parts[5]);
        
        auto op = std::make_unique<DeleteOperation>(pos, length, clientId);
        op->operationId = operationId;
        op->docVersion = docVersion;
        return std::move(op);
    }

    return nullptr;
}
