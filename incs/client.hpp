#ifndef CLIENT_HPP
#define CLIENT_HPP
#pragma once

#include "ircserv.hpp"

class Client {
private:
    int Fd;
    bool authenticated;
    bool passwordValid;
    std::string nickname;
    std::string username;
    std::string realname;
    std::string IPadd;
public:
    Client(const int &fd, const std::string &ipadd);
    Client();
    ~Client();

    int GetFd() const;
    std::string getNickname() const;
    std::string getUsername() const;
    std::string getRealname() const;
    std::string getIpAdd() const;
    bool isAuthenticated() const;
    bool isPasswordValid() const;
    
    void SetNickname(std::string nick);
    void SetUsername(std::string user);
    void SetRealname(std::string real);
    void SetFd(int fd);
    void setIpAdd(std::string ipadd);
    void SetAuthenticated(bool auth);
    void SetPasswordValid(bool valid);
    
    bool hasCompletedRegistration() const;
};

#endif