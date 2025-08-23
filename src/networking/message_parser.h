#pragma once

#include <string>
#include <string_view>

enum class MessageType
{
    UNKNOWN,
    CONNECTED,      // CONNECTED:clientId
    OPERATION,      // INSERT:clientId:operationId:docVersion:pos:text OR DELETE:clientId:operationId:docVersion:pos:length
    INIT_DOCUMENT   // INIT_DOCUMENT:text
};

struct ParsedMessage
{
    MessageType type;
    std::string content;
    std::string clientId;
};

class MessageParser
{
public:
    static ParsedMessage parseMessage(const std::string& msg);
    static std::string createInitDocumentMessage(const std::string& docText);
    static std::string createConnectedMessage(const std::string& clientId);
};
