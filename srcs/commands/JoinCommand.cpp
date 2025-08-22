#include "../../incs/parsing.hpp"
#include "../../incs/server.hpp"
#include "../../incs/client.hpp"
#include "../../incs/channel.hpp"



void Parser::handleJoin(Server *server, const std::string &channelName, const std::string &key, int clientFd) {
    Client* client = server->FindClientByFd(clientFd);
    if (!client) {
        return;
    }

    // Debug output
    std::cout << "DEBUG JOIN HANDLER: channel='" << channelName << "', key='" << key << "'" << std::endl;

    // Check if client is fully registered
    if (!client->hasCompletedRegistration()) {
        server->SendToClient(clientFd, "451 " + client->getNickname() + " :You have not registered\r\n");
        return;
    }

    if (channelName == "0") {
        std::cout << "DEBUG: JOIN 0 - parting from all channels" << std::endl;
        std::vector<Channel*> allChannels = server->GetChannels();

        for (size_t i = 0; i < allChannels.size(); i++) {
            if (allChannels[i]->hasClient(client)) {
                std::string partMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getIpAdd() + " PART " + allChannels[i]->getName() + " :Left all channels\r\n";

                server->SendToClient(clientFd, partMsg);
                allChannels[i]->broadcastMessage(partMsg, client, server);
                allChannels[i]->removeClient(client);
            }
        }

        // Remove empty channels
        server->RemoveEmptyChannels();
        return;
    }

    // Validate channel name
    if (channelName.empty() || !Channel::isValidName(channelName)) {
        server->SendToClient(clientFd, "403 " + client->getNickname() + " " + channelName + " :No such channel\r\n");
        return;
    }

    // Find or create channel
    Channel* channel = server->FindChannelByName(channelName);
    if (!channel) {
        std::cout << "DEBUG: Creating new channel: " << channelName << std::endl;
        channel = server->CreateChannel(channelName);
        if (!channel) {
            server->SendToClient(clientFd, "403 " + client->getNickname() + " " + channelName + " :No such channel\r\n");
            return;
        }
        std::cout << "DEBUG: Channel created successfully" << std::endl;
    } else {
        std::cout << "DEBUG: Found existing channel: " << channelName << " with " << channel->getClientCount() << " clients" << std::endl;
        std::cout << "DEBUG: Current clients in channel: " << channel->getClientsList(server) << std::endl;
    }

    // Check if client is already in channel
    if (channel->hasClient(client)) {
        return; // Already in channel, silently ignore
    }

    // Debug channel key checking
    std::cout << "DEBUG: Channel has key: " << (channel->hasKey() ? "YES" : "NO") << std::endl;
    if (channel->hasKey()) {
        std::cout << "DEBUG: Provided key: '" << key << "'" << std::endl;
        std::cout << "DEBUG: Key check result: " << (channel->checkKey(key) ? "PASS" : "FAIL") << std::endl;
    }

    // Check channel key/password
    if (channel->hasKey() && !channel->checkKey(key)) {
        server->SendToClient(clientFd, "475 " + client->getNickname() + " " + channelName + " :Cannot join channel (+k)\r\n");
        return;
    }

    // Try to add client to channel
    if (!channel->addClient(client)) {
        // Check specific reasons for failure
        if (channel->isBanned(client)) {
            server->SendToClient(clientFd, "474 " + client->getNickname() + " " + channelName + " :Cannot join channel (+b)\r\n");
        } else if (channel->isFull()) {
            server->SendToClient(clientFd, "471 " + client->getNickname() + " " + channelName + " :Cannot join channel (+l)\r\n");
        } else if (channel->isInviteOnly()) {
            server->SendToClient(clientFd, "473 " + client->getNickname() + " " + channelName + " :Cannot join channel (+i)\r\n");
        } else {
            server->SendToClient(clientFd, "473 " + client->getNickname() + " " + channelName + " :Cannot join channel\r\n");
        }
        return;
    }

    // Create JOIN message
    std::string joinMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getIpAdd() + " JOIN " + channelName + "\r\n";

    // Send JOIN confirmation to client
    server->SendToClient(clientFd, joinMsg);

    // Broadcast JOIN to other clients in channel
    channel->broadcastMessage(joinMsg, client, server);

    // Send topic if it exists
    if (!channel->getTopic().empty()) {
        server->SendToClient(clientFd, server->getServerName() + " 332 " + client->getNickname() + " " + channelName + " :" + channel->getTopic() + "\r\n");
    } else {
        server->SendToClient(clientFd, server->getServerName() + " 331 " + client->getNickname() + " " + channelName + " :No topic is set\r\n");
    }

    // Send names list
    std::string namesList = channel->getClientsList(server);
    server->SendToClient(clientFd, server->getServerName() + " 353 " + client->getNickname() + " = " + channelName + " :" + namesList + "\r\n");
    server->SendToClient(clientFd, server->getServerName() + " 366 " + client->getNickname() + " " + channelName + " :End of NAMES list\r\n");

    // Bot greets new user if active (broadcast to everyone after all JOIN protocol is complete)
    if (channel->isBotActive()) {
        std::string greet = ":BOT!bot@server PRIVMSG " + channelName + " :Welcome " + client->getNickname() + "!\r\n";
        channel->broadcastMessage(greet, NULL, server);
        
        // Send private help reminder to the new user
        std::string helpReminder = ":BOT!bot@server PRIVMSG " + client->getNickname() + " :Hi " + client->getNickname() + "! I'm the channel bot. Type !help in the channel to see available commands.\r\n";
        server->SendToClient(clientFd, helpReminder);
    }
}