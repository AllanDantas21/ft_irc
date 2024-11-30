#include "../../incs/server/server.hpp"
#include <unistd.h>
#include <cstring>
#include <sys/socket.h>

Server::Server(int port, int backlog) : port(port), backlog(backlog){
    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd == -1) {
        throw std::runtime_error("Falha ao criar o socket");
    }
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        close(server_fd);
        throw std::runtime_error("Falha ao vincular o socket");
    }
} 

Server::~Server() {
    close(server_fd);
}

void Server::start(){
    if (listen(server_fd, backlog) < 0) {
        throw std::runtime_error("Falha ao colocar o socket em modo de escuta");
    }
    std::cout << "Servidor iniciado na porta " << port << std::endl;
    while (true) {
        int new_socket;
        socklen_t addrlen = sizeof(address);

        new_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (new_socket < 0) {
            std::cerr << "Falha ao aceitar conexão" << std::endl;
            continue;
        }
        std::cout << "Nova conexão aceita!" << std::endl;
        close(new_socket);
    }
}
