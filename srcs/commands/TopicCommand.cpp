#include "../../incs/parsing.hpp"
#include "../../incs/server.hpp"
#include "../../incs/client.hpp"
#include "../../incs/channel.hpp"

void Parser::handleTopic(Server *server, const std::string &channelName, const std::string &topic, int clientFd) {
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

    if (topic.empty()) {
        std::string currentTopic = channel->getTopic();
        if (currentTopic.empty()) {
            server->SendToClient(clientFd, "331 " + client->getNickname() + " " + channelName + " :No topic is set\r\n");
        } else {
            server->SendToClient(clientFd, "332 " + client->getNickname() + " " + channelName + " :" + currentTopic + "\r\n");
        }
        return;
    }

    if (channel->hasMode('t') && !channel->isOperator(client)) {
        server->SendToClient(clientFd, "482 " + client->getNickname() + " " + channelName + " :You're not channel operator\r\n");
        return;
    }

    channel->setTopic(topic, client);
 
    std::string topicMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getIpAdd() + " TOPIC " + channelName + " :" + topic + "\r\n";
    channel->broadcastMessage(topicMsg);
}