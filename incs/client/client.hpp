#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "../ircserv.hpp"

class Client //-> classe para cliente
{
private:
	std::string nickname; //-> nickname do cliente
	std::string username; //-> username do cliente
	int Fd; //-> descritor de arquivo do cliente
	std::string IPadd; //-> endereço IP do cliente
public:
	Client();
	~Client();
	int GetFd();

	void SetNickname(std::string nick);
	void SetUsername(std::string user);
	void SetFd(int fd);
	void setIpAdd(std::string ipadd);
};

#endif