#include "../../incs/parsing.hpp"
#include "../../incs/server.hpp"
#include "../../incs/client.hpp"
#include "../../incs/channel.hpp"

void Parser::handleInvite(Server *server, const std::string &targetNick, const std::string &channelName, int clientFd) {
    Client* client = server->FindClientByFd(clientFd);
    if (!client) {
        return;
    }

    if (!client->hasCompletedRegistration()) {
        server->SendToClient(clientFd, "451 " + client->getNickname() + " :You have not registered\r\n");
        return;
    }
 
    if (targetNick.empty() || channelName.empty()) {
        server->SendToClient(clientFd, "461 " + client->getNickname() + " INVITE :Not enough parameters\r\n");
        return;
    }

    Client* target = server->FindClientByNickname(targetNick);
    if (!target) {
        server->SendToClient(clientFd, "401 " + client->getNickname() + " " + targetNick + " :No such nick\r\n");
        return;
    }
 
    Channel* channel = server->FindChannelByName(channelName);
    if (!channel) {
        server->SendToClient(clientFd, "403 " + client->getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }
 
    if (!channel->hasClient(client)) {
        server->SendToClient(clientFd, "442 " + client->getNickname() + " " + channelName + " :You're not on that channel\r\n");
        return;
    }
 
    if (channel->hasClient(target)) {
        server->SendToClient(clientFd, "443 " + client->getNickname() + " " + targetNick + " " + channelName + " :is already on channel\r\n");
        return;
    }
 
    if (channel->isInviteOnly() && !channel->isOperator(client)) {
        server->SendToClient(clientFd, "482 " + client->getNickname() + " " + channelName + " :You're not channel operator\r\n");
        return;
    }
 
    std::string targetMask = target->getNickname() + "!" + target->getUsername() + "@" + target->getIpAdd();
    channel->addInvite(targetMask);
 
    std::string inviteMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getIpAdd() + " INVITE " + targetNick + " " + channelName + "\r\n";
    server->SendToClient(target->GetFd(), inviteMsg);
    
    server->SendToClient(clientFd, "341 " + client->getNickname() + " " + targetNick + " " + channelName + "\r\n");
}