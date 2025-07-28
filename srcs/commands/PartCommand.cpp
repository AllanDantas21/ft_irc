#include "../../incs/parsing.hpp"
#include "../../incs/server.hpp"
#include "../../incs/client.hpp"
#include "../../incs/channel.hpp"

void Parser::handlePart(Server *server, const std::string &channelName, int clientFd) {
    Client* client = server->FindClientByFd(clientFd);
    if (!client) {
        return;
    }

    if (!client->hasCompletedRegistration()) {
        server->SendToClient(clientFd, "451 " + client->getNickname() + " :You have not registered\r\n");
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

    std::string partMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getIpAdd() + " PART " + channelName + "\r\n";
    channel->broadcastMessage(partMsg, client, server);

    channel->removeClient(client);

    server->RemoveEmptyChannels();
}