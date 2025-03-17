#ifndef CLIENT_HPP
#define CLIENT_HPP
#pragma once

#include "ircserv.hpp"

class Client {
private:
	int Fd;
	bool authenticated;
	std::string nickname;
	std::string username;
	std::string IPadd;
public:
	Client(const int &fd, const std::string &ipadd);
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
	void SetAuthenticated(bool auth);
};

#endif