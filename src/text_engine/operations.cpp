#include "operations.h"

#include <memory>
#include <sstream>
#include <vector>

std::unique_ptr<Operation> Operation::deserialize(const std::string& str)
{
    std::istringstream ss(str);
    std::string token;

    std::vector<std::string> parts;
    while (std::getline(ss, token, ':'))
        parts.push_back(token);

    if (parts.empty())
        return nullptr;

    if (parts[0] == "INSERT")
    {
        if (parts.size() < 4)  // INSERT:clientId:pos:text
            return nullptr;
        
        std::string clientId = parts[1];
        std::size_t pos = std::stoul(parts[2]);
        std::string text = parts[3];
        
        auto op = std::make_unique<InsertOperation>(text, pos, clientId);
        return std::move(op);
    }
    else if (parts[0] == "DELETE")
    {
        if (parts.size() < 4)  // DELETE:clientId:pos:length
            return nullptr;
        
        std::string clientId = parts[1];
        std::size_t pos = std::stoul(parts[2]);
        std::size_t length = std::stoul(parts[3]);
        
        auto op = std::make_unique<DeleteOperation>(pos, length, clientId);
        return std::move(op);
    }
    else if (parts[0] == "CURSOR")
    {
        if (parts.size() < 2)
            return nullptr;
        
        std::size_t pos = std::stoul(parts[1]);
        return std::make_unique<CursorMoveOperation>(pos);
    }

    return nullptr;
}
