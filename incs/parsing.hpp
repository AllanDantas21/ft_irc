#ifndef PARSING_HPP
#define PARSING_HPP

#include "ircserv.hpp"

class Server;

class Parser {
public:
    static void MainParser(Server *server, char *buffer);
    
private:
    // Command handlers
    static void handlePass(Server *server, const std::string &originalMessage, const std::string &password);
    static void handleNick(Server *server, const std::string &originalMessage, const std::string &nickname);
    static void handleUser(Server *server, const std::string &originalMessage, const std::string &username, const std::string &realname);
    
    // Utility functions
    static int findClientFdFromBuffer(Server *server, const std::string &buffer);
};

#endif