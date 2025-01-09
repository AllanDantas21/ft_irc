#include "../../incs/ircserv.hpp"

bool Server::HasSignal = false;

Server::Server() {
    ServerSocketFd = -1;
}

Server::~Server() {}

void Server::AcceptNewClient() {
    Client cli;
    struct sockaddr_in cliadd;
    struct pollfd NewPoll;
    socklen_t len = sizeof(cliadd);

    int incofd = accept(ServerSocketFd, (sockaddr *)&(cliadd), &len);
    if (incofd == -1)
        {std::cout << "accept() falhou" << std::endl; return;}
    if (fcntl(incofd, F_SETFL, O_NONBLOCK) == -1)
        {std::cout << "fcntl() falhou" << std::endl; return;}

    NewPoll.fd = incofd;
    NewPoll.events = POLLIN;
    NewPoll.revents = 0;

    cli.SetFd(incofd);
    cli.setIpAdd(inet_ntoa((cliadd.sin_addr)));
    clients.push_back(cli);
    fds.push_back(NewPoll);

    std::cout << GRE << "Cliente <" << incofd << "> Conectado" << WHI << std::endl;
}

void Server::ReceiveNewData(int fd) {
    char buff[1024];
    memset(buff, 0, sizeof(buff));

    ssize_t bytes = recv(fd, buff, sizeof(buff) - 1 , 0);

    if (bytes <= 0){ //-> verifica se o cliente desconectou
        std::cout << RED << "Cliente <" << fd << "> Desconectado" << WHI << std::endl;
        ClearClients(fd);
        close(fd);
    }
    else {
        buff[bytes] = '\0';
        MainParser(buff);
    }
}

void Server::SignalHandler(int signum) {
    (void)signum;
    std::cout << std::endl << "Sinal Recebido!" << std::endl;
    Server::HasSignal = true;
}

void Server::CloseFds() {
    for (size_t i = 0; i < clients.size(); i++){
        std::cout << RED << "Cliente <" << clients[i].GetFd() << "> Desconectado" << WHI << std::endl;
        close(clients[i].GetFd());
    }
    if (ServerSocketFd != -1){
        std::cout << RED << "Servidor <" << ServerSocketFd << "> Desconectado" << WHI << std::endl;
        close(ServerSocketFd);
    }
}

void Server::SerSocket() {
    struct sockaddr_in add;
    struct pollfd NewPoll;
    add.sin_family = AF_INET; //-> define a família de endereços como IPv4
    add.sin_port = htons(this->Port);
    add.sin_addr.s_addr = INADDR_ANY;

    ServerSocketFd = socket(AF_INET, SOCK_STREAM, 0); //-> cria o socket do servidor
    if (ServerSocketFd == -1)
        throw(std::runtime_error("falha ao criar socket"));

    int en = 1;
    if (setsockopt(ServerSocketFd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1) 
        throw(std::runtime_error("falha ao definir opção (SO_REUSEADDR) no socket"));
    if (fcntl(ServerSocketFd, F_SETFL, O_NONBLOCK) == -1)
        throw(std::runtime_error("falha ao definir opção (O_NONBLOCK) no socket"));
    if (bind(ServerSocketFd, (struct sockaddr *)&add, sizeof(add)) == -1)
        throw(std::runtime_error("falha ao vincular socket"));
    if (listen(ServerSocketFd, SOMAXCONN) == -1)
        throw(std::runtime_error("falha no listen()"));

    NewPoll.fd = ServerSocketFd;
    NewPoll.events = POLLIN;
    NewPoll.revents = 0;
    fds.push_back(NewPoll);
}

void Server::ClearClients(int fd) { //-> limpa os clientes
    for (size_t i = 0; i < fds.size(); i++){
        if (fds[i].fd == fd)
            {fds.erase(fds.begin() + i); break;}
    }
    for (size_t i = 0; i < clients.size(); i++){
        if (clients[i].GetFd() == fd)
            {clients.erase(clients.begin() + i); break;}
    }
}

void Server::ServerInit(int port, std::string password) {
    (void) password;
    this->Port = port;
    SerSocket();

    std::cout << "Esperando conexão...\n";

    while (Server::HasSignal == false){
        if ((poll(&fds[0],fds.size(),-1) == -1) && Server::HasSignal == false)
            throw(std::runtime_error("poll() falhou"));
            
        for (size_t i = 0; i < fds.size(); i++){
            if (fds[i].revents & POLLIN) //-> verifica se há dados para ler
            {
                if (fds[i].fd == ServerSocketFd)
                    AcceptNewClient();
                else
                    ReceiveNewData(fds[i].fd);
            }
        }
    }
    CloseFds();
}
