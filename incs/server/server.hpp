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
#include <cstring>
#include "../client/client.hpp"
//_______________________________________________________//
#define RED "\e[1;31m" 
#define WHI "\e[0;37m"
#define GRE "\e[1;32m"
#define YEL "\e[1;33m"
//______________________________________________________//

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

    void ServerInit(); //-> inicialização do servidor
    void SerSocket(); //-> criação do socket do servidor
    void AcceptNewClient(); //-> aceitar novo cliente
    void ReceiveNewData(int fd); //-> receber novos dados de um cliente registrado

    static void SignalHandler(int signum); //-> manipulador de sinal
 
    void CloseFds(); //-> fechar descritores de arquivo
    void ClearClients(int fd); //-> limpar clientes
};
