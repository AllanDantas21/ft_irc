#ifndef IRCSERV_HPP
#define IRCSERV_HPP

#include <iostream>
#include <vector>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <csignal>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>

#include "parsing.hpp"
#include "client.hpp"
#include "server.hpp"
#include "channel.hpp"
#include "utils/prompt.hpp"

#define RED "\e[1;31m" 
#define WHI "\e[0;37m"
#define GRE "\e[1;32m"
#define YEL "\e[1;33m"

class ArgumentValidator {
public:
    static void validate(int count, char** args);
};

class SignalConfigurator {
public:
    static void configure();
};

class ServerController {
private:
    Server server;

public:
    void execute(int argc, char** argv);

private:
    void validatePort(int portValue);
    void prepareExecution(int argc, char** argv);
    void handleError(const std::exception& e);
    void announceShutdown();
};

#endif