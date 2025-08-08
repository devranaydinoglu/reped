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

    if (parts.empty()) return nullptr;

    if (parts[0] == "INSERT")
    {
        if (parts.size() < 3) return nullptr;
        std::size_t pos = std::stoul(parts[1]);
        char c = parts[2].empty() ? '\0' : parts[2][0];
        return std::make_unique<InsertOperation>(c, pos);
    }
    else if (parts[0] == "DELETE")
    {
        if (parts.size() < 2) return nullptr;
        std::size_t pos = std::stoul(parts[1]);
        return std::make_unique<DeleteOperation>(pos);
    }
    else if (parts[0] == "CURSOR")
    {
        if (parts.size() < 2) return nullptr;
        std::size_t pos = std::stoul(parts[1]);
        return std::make_unique<CursorMoveOperation>(pos);
    }

    return nullptr;
}
