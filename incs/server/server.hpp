#ifndef SERVER_HPP
#define SERVER_HPP

#include "../ircserv.hpp"

class Server
{
private:
    int Port;
    int ServerSocketFd;
    static bool HasSignal;
    std::vector<Client> clients;
    std::vector<struct pollfd> fds;
public:
    Server();
    ~Server();

    void ServerInit(int port, std::string password); //-> inicialização do servidor
    void SerSocket(); //-> criação do socket do servidor
    void AcceptNewClient();
    void ReceiveNewData(int fd);

    static void SignalHandler(int signum);
 
    void CloseFds();
    void ClearClients(int fd);
};

#endif