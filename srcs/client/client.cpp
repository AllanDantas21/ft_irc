#include "../../incs/ircserv.hpp"

Client::Client(){};

int Client::GetFd(){
    return Fd;
}

void Client::SetFd(int fd) {
    Fd = fd;
}

void Client::setIpAdd(std::string ipadd) {
    IPadd = ipadd;
}