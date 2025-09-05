#ifndef PARSING_HPP
#define PARSING_HPP

#include "ircserv.hpp"

class Server;

class Parser {
public:
    static void MainParser(Server *server, char *buffer, int clientFd);
private:
    static void handlePass(Server *server, const std::string &password, int clientFd);
    static void handleNick(Server *server, const std::string &nickname, int clientFd);
    static void handleUser(Server *server, const std::string &username, const std::string &realname, int clientFd);
    static void handlePrivmsg(Server *server, const std::string &target, const std::string &message, int clientFd);
    static void handleJoin(Server *server, const std::string &channelName, const std::string &key, int clientFd);
    static void handlePart(Server *server, const std::string &channelName, int clientFd);
    static void handleTopic(Server *server, const std::string &channelName, const std::string &topic, int clientFd);
    static void handleKick(Server *server, const std::string &channelName, const std::string &targetNick, const std::string &reason, int clientFd);
    static void handleInvite(Server *server, const std::string &targetNick, const std::string &channelName, int clientFd);
    static void handleMode(Server *server, const std::string &target, const std::string &modes, const std::string &params, int clientFd);
    static void handleDccSend(Server *server, const std::string &target, const std::string &filename, int clientFd);
    static void handleDccGet(Server *server, const std::string &targetNick,const std::string &filename, int clientFd);
};

#endif
