#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../ircserv.hpp"

class Client //-> classe para cliente
{
private:
	int Fd; //-> descritor de arquivo do cliente
	std::string IPadd; //-> endereço IP do cliente
public:
	Client(); //-> construtor padrão
	int GetFd();//-> getter para fd

	void SetFd(int fd); //-> setter para fd
	void setIpAdd(std::string ipadd); //-> setter para ipadd
};

#endif