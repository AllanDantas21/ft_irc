#include "../incs/client.hpp"
#include "../incs/ircserv.hpp"

Client::Client(const int &fd, const std::string &ipadd) : Fd(fd), IPadd(ipadd) {
    this->authenticated = false;
    this->passwordValid = false;
}

Client::Client() : Fd(-1) {
    this->authenticated = false;
    this->passwordValid = false;
}

Client::~Client(){};

int Client::GetFd() const { return Fd; }
std::string Client::getNickname() const { return nickname; }
std::string Client::getUsername() const { return username; }
std::string Client::getRealname() const { return realname; }
std::string Client::getIpAdd() const { return IPadd; }
bool Client::isAuthenticated() const { return authenticated; }
bool Client::isPasswordValid() const { return passwordValid; }

void Client::SetFd(int fd) { Fd = fd; }
void Client::setIpAdd(std::string ipadd) { IPadd = ipadd; }
void Client::SetNickname(std::string nick) { this->nickname = nick; }
void Client::SetUsername(std::string user) { this->username = user; }
void Client::SetRealname(std::string real) { this->realname = real; }
void Client::SetAuthenticated(bool auth) { this->authenticated = auth; }
void Client::SetPasswordValid(bool valid) { this->passwordValid = valid; }

bool Client::hasCompletedRegistration() const {
    return passwordValid && !nickname.empty() && !username.empty();
}