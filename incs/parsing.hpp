#ifndef PARSING_HPP
#define PARSING_HPP

#include "ircserv.hpp"

class Server;

class Parser {
public:
    static void MainParser(Server *server, char *buffer);
private:
};

#endif