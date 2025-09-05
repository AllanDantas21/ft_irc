#ifndef SERVER_HPP
#define SERVER_HPP
#pragma once

#include "ircserv.hpp"
#include "channel.hpp"
#include "DccServer.hpp"
#include "DccClient.hpp"
#include <cerrno>
#include <map>
#include <deque>

class Client;
class Channel;
class DccServer;
class DccClient;

class Server
{
private:
    int Port;
    int ServerSocketFd;
    static bool HasSignal;
    std::string password;
    std::string serverName;
    std::vector<Client> clients;
    std::vector<struct pollfd> fds;
    std::vector<Channel*> channels;
    std::vector<DccServer*> dccServers;
    std::vector<DccClient*> dccClients;
    std::map<int, std::deque<std::string> > outQueues;
    std::map<int, size_t> outOffsets;
    std::map<int, std::string> inBuffers;

public:
    Server();
    ~Server();

    void ServerInit(int port, std::string password);
    void ServerConfig();
    void WaitConnection();
    void AcceptNewClient();
    void ReceiveNewData(int fd);

    static void SignalHandler(int signum);

    void CloseFds();
    void CloseFd(int fd);
    void CloseClientFd(int fd);
    void ClearClients(int fd);
    std::string getServerName() const;

    void SetFd(int fd);
    void SetPort(int port);
    void SetPassword(const std::string &password);
    void SetAtributes(int port, std::string password);

    void SetupSocketOptions();
    void BindAndListenSocket(struct sockaddr_in &add);
    void HandlePollEvents();
    void FlushClient(int fd);
    void EnablePOLLOUT(int fd);
    void DisablePOLLOUT(int fd);
    void AppendAndParseClientInput(int fd, const char* data, size_t length);
    void ParseBufferedLines(int fd, std::vector<std::string>& outLines);
    void ProcessClientLine(std::string& line);

    Client* FindClientByFd(int fd);
    Client* FindClientByNickname(const std::string &nickname);
    const std::vector<Client>& GetClients() const;
    bool CheckPassword(const std::string &password);
    bool IsNicknameInUse(const std::string &nickname);
    void CheckClientAuthentication(Client* client);
    void SendToClient(int fd, const std::string& message);
    void QueueSend(int fd, const std::string& message);
    void SendWelcomeMessage(int fd);
    void SendRegistrationCompleteMessage(Client* client);

    Channel* FindChannelByName(const std::string& name);
    Channel* CreateChannel(const std::string& name);
    bool RemoveChannel(const std::string& name);
    void RemoveEmptyChannels();
    std::vector<Channel*> GetChannels() const;
    void HandleDccEvents(int fd);
    void addPollFd(const struct pollfd& NewFd);
    void addDccServer(DccServer* NewDccServer);
    void addDccClient(DccClient* NewDccClient);
};

#endif
