#ifndef SERVER_HPP
#define SERVER_HPP

#include "../ircserv.hpp"

class Server
{
private:
    int Port;
    int ServerSocketFd;
    static bool HasSignal; //-> booleano estático para sinal
    std::vector<Client> clients; //-> vetor de clientes
    std::vector<struct pollfd> fds; //-> vetor de pollfd
public:
    Server(){ServerSocketFd = -1;} //-> construtor padrão

    void ServerInit(int port, std::string password); //-> inicialização do servidor
    void SerSocket(); //-> criação do socket do servidor
    void AcceptNewClient(); //-> aceitar novo cliente
    void ReceiveNewData(int fd); //-> receber novos dados de um cliente registrado

    static void SignalHandler(int signum); //-> manipulador de sinal
 
    void CloseFds(); //-> fechar descritores de arquivo
    void ClearClients(int fd); //-> limpar clientes
};

#endif