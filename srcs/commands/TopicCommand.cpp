#include "../../incs/parsing.hpp"
#include "../../incs/server.hpp"
#include "../../incs/client.hpp"
#include "../../incs/channel.hpp"

void Parser::handleTopic(Server *server, const std::string &channelName, const std::string &topic, int clientFd) {
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
    
    // If no topic provided, return current topic
    if (topic.empty()) {
        std::string currentTopic = channel->getTopic();
        if (currentTopic.empty()) {
            server->SendToClient(clientFd, "331 " + client->getNickname() + " " + channelName + " :No topic is set\r\n");
        } else {
            server->SendToClient(clientFd, "332 " + client->getNickname() + " " + channelName + " :" + currentTopic + "\r\n");
        }
        return;
    }
    
    // Check if topic is restricted to operators
    if (channel->hasMode('t') && !channel->isOperator(client)) {
        server->SendToClient(clientFd, "482 " + client->getNickname() + " " + channelName + " :You're not channel operator\r\n");
        return;
    }
    
    // Set new topic
    channel->setTopic(topic, client);
    
    // Broadcast topic change to all clients in channel
    std::string topicMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getIpAdd() + " TOPIC " + channelName + " :" + topic + "\r\n";
    channel->broadcastMessage(topicMsg);
}