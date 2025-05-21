#include "../incs/ircserv.hpp"
#include <sstream>

void Parser::MainParser(Server *server, char *buffer) {
    std::string buff(buffer);
    std::string command;
    std::istringstream iss(buff);
    
    iss >> command;
    
    if (command == "PASS") {
        std::string password;
        iss >> password;
        handlePass(server, iss.tellg() > 0 ? buff.substr(iss.tellg()) : "", password);
    } 
    else if (command == "NICK") {
        std::string nickname;
        iss >> nickname;
        handleNick(server, buff, nickname);
    } 
    else if (command == "USER") {
        std::string username, hostname, servername, realname;
        iss >> username >> hostname >> servername;
        
        size_t pos = buff.find(':', 0);
        if (pos != std::string::npos) {
            realname = buff.substr(pos + 1);
        }
        
        handleUser(server, buff, username, realname);
    } 
    else {
        int clientFd = findClientFdFromBuffer(server, buff);
        if (clientFd != -1) {
            server->SendToClient(clientFd, "Command not implemented yet: " + buff);
        }
    }
}