#include "../incs/ircserv.hpp"

bool Server::HasSignal = false;

Server::Server() : ServerSocketFd(-1) {}
Server::~Server() {}

void Server::ServerInit(int port, std::string password) {
    this->SetAtributes(port, password);
    this->ServerConfig();

    std::cout << "Iniciando servidor na porta " << port << "..." << std::endl;
    std::cout << "Esperando conexão...\n";

    this->WaitConnection();
    this->CloseFds();
}

void Server::SetAtributes(int port, std::string password) {
    this->SetPort(port);
    this->SetPassword(password);
}

void setupPollFd(int incofd, struct pollfd &NewPoll)
{
    NewPoll.fd = incofd;
    NewPoll.events = POLLIN;
    NewPoll.revents = 0;
}

void Server::ServerConfig() {
    struct sockaddr_in add;
    struct pollfd NewPoll;
    add.sin_family = AF_INET;
    add.sin_port = htons(this->Port);
    add.sin_addr.s_addr = INADDR_ANY;

    ServerSocketFd = socket(AF_INET, SOCK_STREAM, 0);
    if (ServerSocketFd == -1)
        throw(std::runtime_error("falha ao criar socket"));

    SetupSocketOptions();
    BindAndListenSocket(add);

    setupPollFd(ServerSocketFd, NewPoll);
    fds.push_back(NewPoll);
}

void Server::WaitConnection() {
    while (Server::HasSignal == false) {
        if ((poll(&fds[0], fds.size(), -1) == -1) && Server::HasSignal == false)
            throw(std::runtime_error("poll() falhou"));
        this->HandlePollEvents();
    }
}

void Server::HandlePollEvents() {
    for (size_t i = 0; i < fds.size(); i++) {
        if (fds[i].revents & POLLIN) {
            if (fds[i].fd == ServerSocketFd)
                AcceptNewClient();
            else
                ReceiveNewData(fds[i].fd);
        }
    }
}

void Server::CloseFds() {
    for (size_t i = 0; i < clients.size(); i++) {
        std::cout << RED << "Cliente <" << clients[i].GetFd() << "> Desconectado" << WHI << std::endl;
        close(clients[i].GetFd());
    }
    if (ServerSocketFd != -1) {
        std::cout << RED << "Servidor <" << ServerSocketFd << "> Desconectado" << WHI << std::endl;
        close(ServerSocketFd);
    }
}

static bool validateNewConnection(int socketFd) {
    if (socketFd == -1)
        return(std::cout << "accept() falhou" << std::endl, false);
    if (fcntl(socketFd, F_SETFL, O_NONBLOCK) == -1)
        return(std::cout << "fcntl() falhou" << std::endl, false);
    return true;
}

static void connectClient(int incofd, struct sockaddr_in cliadd, std::vector<Client> &clients, std::vector<struct pollfd> &fds) {
    struct pollfd NewPoll;
    setupPollFd(incofd, NewPoll);
    Client cli(NewPoll.fd, inet_ntoa((cliadd.sin_addr)));
    clients.push_back(cli);
    fds.push_back(NewPoll);
    std::cout << GRE << "Cliente <" << incofd << "> Conectado" << WHI << std::endl;
}

void Server::AcceptNewClient() {
    struct sockaddr_in cliadd;
    socklen_t len = sizeof(cliadd);
    memset(&cliadd, 0, sizeof(cliadd));

    int incofd = accept(ServerSocketFd, (sockaddr *)&(cliadd), &len);
    if (!validateNewConnection(incofd))
        return ;

    connectClient(incofd, cliadd, clients, fds);
    this->SendWelcomeMessage(incofd);
}

void Server::ReceiveNewData(int fd) {
    char buff[1024];

    memset(buff, 0, sizeof(buff));
    ssize_t bytes = recv(fd, buff, sizeof(buff) - 1, 0);

    if (bytes <= 0) {
        std::cout << RED << "Cliente <" << fd << "> Desconectado" << WHI << std::endl;
        ClearClients(fd);
        return (close(fd), void());
    }
    Parser::MainParser(this, buff);
}

void Server::SignalHandler(int signum) {
    (void)signum;
    std::cout << std::endl << "Sinal Recebido!" << std::endl;
    Server::HasSignal = true;
}

void Server::SetupSocketOptions() {
    int en = 1;
    if (setsockopt(ServerSocketFd, SOL_SOCKET, SO_REUSEADDR, &en, sizeof(en)) == -1) 
        throw(std::runtime_error("falha ao definir opção (SO_REUSEADDR) no socket"));
    if (fcntl(ServerSocketFd, F_SETFL, O_NONBLOCK) == -1)
        throw(std::runtime_error("falha ao definir opção (O_NONBLOCK) no socket"));
}

void Server::BindAndListenSocket(struct sockaddr_in &add) {
    if (bind(ServerSocketFd, (struct sockaddr *)&add, sizeof(add)) == -1)
        throw(std::runtime_error("falha ao vincular socket"));
    if (listen(ServerSocketFd, SOMAXCONN) == -1)
        throw(std::runtime_error("falha no listen()"));
}

void Server::ClearClients(int fd) {
    CloseClientFd(fd);
    CloseFd(fd);
}

void Server::CloseClientFd(int fd) {
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i].GetFd() == fd) {
            close(clients[i].GetFd());
            break;
        }
    }
}

void Server::CloseFd(int fd) {
    for (size_t i = 0; i < fds.size(); i++) {
        if (fds[i].fd == fd) {
            fds.erase(fds.begin() + i);
            break;
        }
    }
}

void Server::SendToClient(int fd, const std::string& message)
{
    send(fd, message.c_str(), message.length(), 0);
}

void Server::SendWelcomeMessage(int fd)
{
    std::string welcomeMsg = "Bem vindo ao nosso IRC Server!\r\nPor favor, autentique-se com o comando PASS.\r\n";
    SendToClient(fd, welcomeMsg);
}

void Server::SetFd(int fd) { this->ServerSocketFd = fd; }
void Server::SetPort(int port) { this->Port = port; }
void Server::SetPassword(const std::string &password) { this->password = password; }
