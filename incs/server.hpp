#ifndef SERVER_HPP
#define SERVER_HPP
#pragma once

#include "ircserv.hpp"

class Client;

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
    void ServerConfig();
    void WaitConnection();
    void AcceptNewClient();
    void ReceiveNewData(int fd);

    static void SignalHandler(int signum);
 
    void CloseFds();
    void CloseFd(int fd);
    void CloseClientFd(int fd);
    void ClearClients(int fd);
    
    void SetFd(int fd);
    void SetPort(int port);
    void SetPassword(const std::string &password);
    void SetAtributes(int port, std::string password);

    void SetupSocketOptions();
    void BindAndListenSocket(struct sockaddr_in &add);
    void HandlePollEvents();

    Client* FindClientByFd(int fd);
    void HandleAuthentication(Client* client, const char* message);
    void SendToClient(int fd, const std::string& message);
    void SendWelcomeMessage(int fd);
};

#endif