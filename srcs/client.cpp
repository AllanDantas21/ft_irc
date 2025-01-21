#include "../incs/ircserv.hpp"

Client::Client(){};
Client::~Client(){};

int Client::GetFd() { return Fd; }
void Client::SetFd(int fd) { Fd = fd; }
void Client::setIpAdd(std::string ipadd) { IPadd = ipadd; }
void Client::SetNickname(std::string nick) { this->nickname = nick; }
void Client::SetUsername(std::string user) { this->username = user; }