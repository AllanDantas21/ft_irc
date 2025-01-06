#include <iostream>
#include <vector> //-> para vector
#include <sys/socket.h> //-> para socket()
#include <sys/types.h> //-> para socket()
#include <netinet/in.h> //-> para sockaddr_in
#include <fcntl.h> //-> para fcntl()
#include <unistd.h> //-> para close()
#include <arpa/inet.h> //-> para inet_ntoa()
#include <poll.h> //-> para poll()
#include <csignal> //-> para signal()
//_______________________________________________________//
#define RED "\e[1;31m" //-> para cor vermelha
#define WHI "\e[0;37m" //-> para cor branca
#define GRE "\e[1;32m" //-> para cor verde
#define YEL "\e[1;33m" //-> para cor amarela

class Client //-> class for client
{
private:
	int Fd; //-> client file descriptor
	std::string IPadd; //-> client ip address
public:
	Client(){}; //-> default constructor
	int GetFd(){return Fd;} //-> getter for fd

	void SetFd(int fd){Fd = fd;} //-> setter for fd
	void setIpAdd(std::string ipadd){IPadd = ipadd;} //-> setter for ipadd
};
