#include "../../incs/server/server.hpp"

bool Server::HasSignal = false;

void Server::SignalHandler(int signum)
{
    (void)signum;
    std::cout << std::endl << "Sinal Recebido!" << std::endl;
    Server::HasSignal = true;
}

void Server::CloseFds(){
    for(size_t i = 0; i < clients.size(); i++){
        std::cout << RED << "Cliente <" << clients[i].GetFd() << "> Desconectado" << WHI << std::endl;
        close(clients[i].GetFd());
    }
    if (ServerSocketFd != -1){
        std::cout << RED << "Servidor <" << ServerSocketFd << "> Desconectado" << WHI << std::endl;
        close(ServerSocketFd);
    }
}

void Server::SerSocket()
{
    struct sockaddr_in add;
    struct pollfd NewPoll;
    add.sin_family = AF_INET; //-> define a família de endereços como IPv4
    add.sin_port = htons(this->Port);
    add.sin_addr.s_addr = INADDR_ANY;

    ServerSocketFd = socket(AF_INET, SOCK_STREAM, 0); //-> cria o socket do servidor
    if(ServerSocketFd == -1)
        throw(std::runtime_error("falha ao criar socket"));

    int en = 1;
    if(setsockopt(ServerSocketFd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1) 
        throw(std::runtime_error("falha ao definir opção (SO_REUSEADDR) no socket"));
    if (fcntl(ServerSocketFd, F_SETFL, O_NONBLOCK) == -1)
        throw(std::runtime_error("falha ao definir opção (O_NONBLOCK) no socket"));
    if (bind(ServerSocketFd, (struct sockaddr *)&add, sizeof(add)) == -1)
        throw(std::runtime_error("falha ao vincular socket"));
    if (listen(ServerSocketFd, SOMAXCONN) == -1)
        throw(std::runtime_error("falha no listen()"));

    NewPoll.fd = ServerSocketFd; //-> adiciona o socket do servidor ao pollfd
    NewPoll.events = POLLIN;
    NewPoll.revents = 0;
    fds.push_back(NewPoll);
}

void Server::ClearClients(int fd){ //-> limpa os clientes
    for(size_t i = 0; i < fds.size(); i++){ //-> remove o cliente do pollfd
        if (fds[i].fd == fd)
            {fds.erase(fds.begin() + i); break;}
    }
    for(size_t i = 0; i < clients.size(); i++){ //-> remove o cliente do vetor de clientes
        if (clients[i].GetFd() == fd)
            {clients.erase(clients.begin() + i); break;}
    }
}

void Server::ServerInit()
{
    this->Port = 4444;
    SerSocket(); //-> cria o socket do servidor

    std::cout << GRE << "Servidor <" << ServerSocketFd << "> Conectado" << WHI << std::endl;
    std::cout << "Aguardando para aceitar uma conexão...\n";
}