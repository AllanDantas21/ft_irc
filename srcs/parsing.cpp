#include "../incs/ircserv.hpp"
#include <sstream>

void Parser::MainParser(Server *server, char *buffer, int clientFd) {
    std::string buff(buffer);
    std::string command;
    std::istringstream iss(buff);
    
    iss >> command;
    
    if (command == "PASS") {
        std::string password;
        iss >> password;
        handlePass(server, password, clientFd);
    } 
    else if (command == "NICK") {
        std::string nickname;
        iss >> nickname;
        handleNick(server, nickname, clientFd);
    } 
    else if (command == "USER") {
        std::string username, hostname, servername, realname;
        iss >> username >> hostname >> servername;
        
        size_t pos = buff.find(':', 0);
        if (pos != std::string::npos) {
            realname = buff.substr(pos + 1);
        }
        
        handleUser(server, username, realname, clientFd);
    }
    else if (command == "PRIVMSG") {
        std::string target, message;
        iss >> target;
        
        size_t pos = buff.find(':', 0);
        if (pos != std::string::npos) {
            message = buff.substr(pos + 1);
        }
        
        handlePrivmsg(server, target, message, clientFd);
    } 
    else {
        server->SendToClient(clientFd, "Command not implemented yet: " + buff);
    }
}