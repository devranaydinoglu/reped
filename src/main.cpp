#include <iostream>
#include "networking/server.h"
#include "networking/client.h"

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        std::cout << "Expected arguments: [server|client] [server_address]\n";
        return 1;
    }

    std::string mode = argv[1];
    
    if (mode == "server")
    {
        std::cout << "Starting server...\n";

        Server server;
        
        std::cout << "Press Enter to stop the server..." << std::endl;
        std::cin.get();        
    }
    else if (mode == "client")
    {
        std::string serverAddress = argc > 2 ? argv[2] : "localhost";
        std::cout << "Connecting to server at " << serverAddress << "...\n";
        
        Client client(serverAddress);
        
        if (!client.isConnected())
        {
            std::cout << "Failed to connect to server.\n";
            return 1;
        }

        client.setMessageReceivedCallback([](const std::string& message)
        {
            std::cout << "Received: " << message << "\n";
        });

        std::string input;
        while (true)
        {
            std::getline(std::cin, input);
            if (input == "quit") break;
            
            if (!client.sendMessage(input))
            {
                std::cout << "Failed to send message.\n";
                break;
            }
        }
    }
    else
    {
        std::cout << "Invalid mode. Use 'server' or 'client'.\n";
        return 1;
    }
}
