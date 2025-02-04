#include "../incs/ircserv.hpp"
#include <sys/socket.h>
#include <unistd.h>

Client::Client(){};
Client::~Client(){};

int Client::GetFd() { return Fd; }
std::string Client::getNickname() { return nickname; }
std::string Client::getUsername() { return username; }
std::string Client::getIpAdd() { return IPadd; }
void Client::SetFd(int fd) { Fd = fd; }
void Client::setIpAdd(std::string ipadd) { IPadd = ipadd; }
void Client::SetNickname(std::string nick) { this->nickname = nick; }
void Client::SetUsername(std::string user) { this->username = user; }

void Client::SendMessage(const std::string &message) {
    if (send(Fd, message.c_str(), message.length(), 0) == -1) {
        std::cerr << "Error sending message to client" << std::endl;
    }
}