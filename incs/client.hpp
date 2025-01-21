#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "ircserv.hpp"

class Client {
private:
	std::string nickname; //-> nickname
	std::string username; //-> username
	std::string IPadd; //-> endereço IP
	int Fd;
public:
	Client();
	~Client();

	int GetFd();
	std::string getNickname();
	std::string getUsername();
	std::string getIpAdd();
	void SetNickname(std::string nick);
	void SetUsername(std::string user);
	void SetFd(int fd);
	void setIpAdd(std::string ipadd);
};

#endif