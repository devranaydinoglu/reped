#include <sstream>
#include <vector>

#include "message_parser.h"

ParsedMessage MessageParser::parseMessage(const std::string& msg)
{
    ParsedMessage parsedMsg;
    parsedMsg.content = msg;

    std::istringstream iss(msg);
    std::string token;
    std::vector<std::string> parts;

    while (std::getline(iss, token, ':'))
        parts.push_back(token);
    
    if (parts.empty())
    {
        parsedMsg.type = MessageType::UNKNOWN;
        parsedMsg.clientId = "UNKNOWN";
        return parsedMsg;
    }

    if (parts[0] == "CONNECTED")
    {
        parsedMsg.type = MessageType::CONNECTED;
        parsedMsg.clientId = parts[1];
        return parsedMsg;
    }

    if (parts[0] == "INIT_DOCUMENT")
    {
        parsedMsg.type = MessageType::INIT_DOCUMENT;
        parsedMsg.clientId = "UNKNOWN";
        return parsedMsg;
    }

    if (parts[0] == "INSERT" || parts[0] == "DELETE")
    {
        parsedMsg.type = MessageType::OPERATION;
        parsedMsg.clientId = parts[1];
        return parsedMsg;
    }

    parsedMsg.type = MessageType::UNKNOWN;
    parsedMsg.clientId = "UNKNOWN";
    return parsedMsg;
}

std::string MessageParser::createInitDocumentMessage(const std::string& docText)
{
    return "INIT_DOCUMENT:" + docText;
}

std::string MessageParser::createConnectedMessage(const std::string& clientId)
{
    return "CONNECTED:" + clientId;
}
