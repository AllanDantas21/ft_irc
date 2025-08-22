#include "../../incs/ircserv.hpp"
bool isValidOperation(Server *server, Client *sender, const std::string &target, const std::string &message, int clientFd);

void Parser::handlePrivmsg(Server *server, const std::string &target, const std::string &message, int clientFd) {
    Client* sender = server->FindClientByFd(clientFd);
    if (!sender) return;

    if (!isValidOperation(server, sender, target, message, clientFd)) {
        return;
    }

    if (target[0] == '#' || target[0] == '&') {
        Channel* channel = server->FindChannelByName(target);
        if (!channel) {
            server->SendToClient(clientFd, "403 " + sender->getNickname() + " " + target + " :No such channel\r\n");
            return;
        }
        
        if (!channel->hasClient(sender)) {
            server->SendToClient(clientFd, "404 " + sender->getNickname() + " " + target + " :Cannot send to channel\r\n");
            return;
        }
        
        std::string formattedMessage = ":" + sender->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
        channel->broadcastMessage(formattedMessage, sender, server);
        // Bot stores last 100 messages
        if (channel->isBotActive()) {
            channel->storeMessage("[" + sender->getNickname() + "] " + message);
            // Bot responds to !datenow
            if (message == "!datenow") {
                time_t now = time(0);
                struct tm *tm_info = localtime(&now);
                char dateStr[64];
                strftime(dateStr, sizeof(dateStr), "%Y-%m-%d %H:%M:%S", tm_info);
                std::string dateMsg = ":BOT!bot@server PRIVMSG " + target + " :Current date and time: " + dateStr + "\r\n";
                channel->broadcastMessage(dateMsg, NULL, server);
            }
            // Bot responds to !history
            if (message == "!history") {
                std::deque<std::string> history = channel->getLastMessages();
                if (history.empty()) {
                    std::string noMsg = ":BOT!bot@server PRIVMSG " + sender->getNickname() + " :No history available.\r\n";
                    channel->broadcastMessage(noMsg, NULL, server);
                } else {
                    for (std::deque<std::string>::const_iterator it = history.begin(); it != history.end(); ++it) {
                        std::string histMsg = ":BOT!bot@server PRIVMSG " + sender->getNickname() + " :" + *it + "\r\n";
                        channel->broadcastMessage(histMsg, NULL, server);
                    }
                }
            }
        }
    } else {
        Client* recipient = server->FindClientByNickname(target);
        if (!recipient) {
            server->SendToClient(clientFd, "401 " + sender->getNickname() + " " + target + " :No such nick\r\n");
            return;
        }
        
        if (!recipient->hasCompletedRegistration()) {
            server->SendToClient(clientFd, "401 " + sender->getNickname() + " " + target + " :No such nick\r\n");
            return;
        }
        
        std::string formattedMessage = ":" + sender->getNickname() + " PRIVMSG " + target + " :" + message + "\r\n";
        server->SendToClient(recipient->GetFd(), formattedMessage);
    }
}


bool isValidOperation(Server *server, Client *sender, const std::string &target, const std::string &message, int clientFd) {
    if (!sender->hasCompletedRegistration()) {
        server->SendToClient(clientFd, "451 :You have not registered\r\n");
        return false;
    }
    
    if (target.empty()) {
        server->SendToClient(clientFd, "411 :No recipient given (PRIVMSG)\r\n");
        return false;
    }
    
    if (message.empty()) {
        server->SendToClient(clientFd, "412 :No text to send\r\n");
        return false;
    }
    return true;
}