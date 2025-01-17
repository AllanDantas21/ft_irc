#ifndef IRCSERV_HPP
#define IRCSERV_HPP

#include <iostream>
#include <vector>
#include <sys/socket.h> //-> para socket()
#include <sys/types.h> //-> para socket()
#include <netinet/in.h> //-> para sockaddr_in
#include <fcntl.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>
#include <csignal>
#include <cstdlib>
#include <cstring>

#include "parsing/parsing.hpp"
#include "client/client.hpp"
#include "server/server.hpp"
#include "utils/prompt.hpp"

//_______________________________________________________//
#define RED "\e[1;31m" 
#define WHI "\e[0;37m"
#define GRE "\e[1;32m"
#define YEL "\e[1;33m"
//______________________________________________________//

#endif