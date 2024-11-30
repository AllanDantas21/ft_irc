#ifndef SERVER_HPP
#define SERVER_HPP

#include <iostream>
#include <string>
#include <netinet/in.h>

class Server {
private:
    int                 server_fd;
    struct sockaddr_in  address;
    int                 port;
    int                 backlog;

public:
    Server(int port, int backlog = 10);
    ~Server();

    void start();
};

#endif
