#include "../incs/ircserv.hpp"

void MainParser(char *buffer, Server &server, Client &client) {
    std::string buff(buffer);
    std::cout << "command received: " << buff << std::endl;
    
    if (buff.compare(0, 4, "PASS") == 0) {
        std::istringstream iss(buff);
        std::string command, inputPass;
        iss >> command >> inputPass;

        std::string response;
        if (inputPass == server.GetPassword()) {
            response = "valid password. client authenticated!\n";
        } else {
            response = "invalid password. access denied!\n";
        }
        client.SendMessage(response);
    }
}