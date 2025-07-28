#pragma once

#include "client.hpp"
#include <vector>
#include <string>
#include <map>

class Client;
class Server;

class Channel {
private:
    std::vector<int> clientFds;
    std::vector<int> operatorFds;
    std::string channelName;
    std::string topic;
    std::string key;
    std::map<char, bool> modes;
    int userLimit;
    std::vector<std::string> inviteList;
    std::vector<std::string> banList;

public:
    Channel(std::string name);
    ~Channel();

    static bool isValidName(const std::string& attemptName);
    std::string getName() const;

    bool addClient(Client* client);
    bool removeClient(Client* client);
    bool hasClient(Client* client) const;
    bool isOperator(Client* client) const;
    bool addOperator(Client* client);
    bool removeOperator(Client* client);

    void broadcastMessage(const std::string& message, Client* sender, Server* server);
    

    void setTopic(const std::string& newTopic, Client* setter = NULL);
    std::string getTopic() const;
    

    void setMode(char mode, bool value);
    bool hasMode(char mode) const;
    std::string getModes() const;
    

    bool isInviteOnly() const;
    bool hasKey() const;
    bool checkKey(const std::string& providedKey) const;
    void setKey(const std::string& newKey);
    bool isBanned(Client* client) const;
    void addBan(const std::string& mask);
    void removeBan(const std::string& mask);

    void addInvite(const std::string& mask);
    void removeInvite(const std::string& mask);

    void setUserLimit(int limit);
    int  getUserLimit() const;
    bool isFull() const;

    int getClientCount() const;
    std::vector<Client*> getClients() const;
    std::vector<Client*> getOperators() const;
    std::string getClientsList(Server* server) const;
};