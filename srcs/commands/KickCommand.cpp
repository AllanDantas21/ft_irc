#include "../../incs/parsing.hpp"
#include "../../incs/server.hpp"
#include "../../incs/client.hpp"
#include "../../incs/channel.hpp"

void Parser::handleKick(Server *server, const std::string &channelName, const std::string &targetNick, const std::string &reason, int clientFd) {
    Client* client = server->FindClientByFd(clientFd);
    if (!client) {
        return;
    }
    
    // Check if client is fully registered
    if (!client->hasCompletedRegistration()) {
        server->SendToClient(clientFd, "451 " + client->getNickname() + " :You have not registered\r\n");
        return;
    }
    
    // Find channel
    Channel* channel = server->FindChannelByName(channelName);
    if (!channel) {
        server->SendToClient(clientFd, "403 " + client->getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }
    
    // Check if client is in channel
    if (!channel->hasClient(client)) {
        server->SendToClient(clientFd, "442 " + client->getNickname() + " " + channelName + " :You're not on that channel\r\n");
        return;
    }
    
    // Check if client is operator
    if (!channel->isOperator(client)) {
        server->SendToClient(clientFd, "482 " + client->getNickname() + " " + channelName + " :You're not channel operator\r\n");
        return;
    }
    
    // Find target client
    Client* target = server->FindClientByNickname(targetNick);
    if (!target) {
        server->SendToClient(clientFd, "401 " + client->getNickname() + " " + targetNick + " :No such nick\r\n");
        return;
    }
    
    // Check if target is in channel
    if (!channel->hasClient(target)) {
        server->SendToClient(clientFd, "441 " + client->getNickname() + " " + targetNick + " " + channelName + " :They aren't on that channel\r\n");
        return;
    }
    
    // Create kick message
    std::string kickMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getIpAdd() + " KICK " + channelName + " " + targetNick;
    if (!reason.empty()) {
        kickMsg += " :" + reason;
    }
    kickMsg += "\r\n";
    
    // Send KICK message to all clients in channel
    channel->broadcastMessage(kickMsg, client, server);
    
    // Remove target from channel
    channel->removeClient(target);
    
    // Remove empty channels
    server->RemoveEmptyChannels();
}