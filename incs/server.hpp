#ifndef SERVER_HPP
#define SERVER_HPP

#include "ircserv.hpp"

class Server
{
private:
    int Port;
    int ServerSocketFd;
    static bool HasSignal;
    std::string password;
    std::vector<Client> clients;
    std::vector<struct pollfd> fds;
public:
    Server();
    ~Server();

    void ServerInit(int port, std::string password);
    void SerSocket();
    void AcceptNewClient();
    void ReceiveNewData(int fd);

    static void SignalHandler(int signum);
 
    void CloseFds();
    void ClearClients(int fd);
    
    void SetFd(int fd);
    void SetPort(int port);
    void SetPassword(const std::string &password);

    void SetupSocketOptions();
    void BindAndListenSocket(struct sockaddr_in &add);
    void HandlePollEvents();
};

#endif