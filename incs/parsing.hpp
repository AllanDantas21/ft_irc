#ifndef PARSING_HPP
#define PARSING_HPP

#include "ircserv.hpp"

class Server;

class Parser {
public:
    static void MainParser(Server *server, char *buffer, int clientFd);
private:
    static void handlePass(Server *server, const std::string &password, int clientFd);
    static void handleNick(Server *server, const std::string &nickname, int clientFd);
    static void handleUser(Server *server, const std::string &username, const std::string &realname, int clientFd);
};

#endif