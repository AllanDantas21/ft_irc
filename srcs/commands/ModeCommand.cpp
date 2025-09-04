#include "../../incs/parsing.hpp"
#include "../../incs/server.hpp"
#include "../../incs/client.hpp"
#include "../../incs/channel.hpp"
#include <sstream>


static bool validateModeRequest(Server* server, Channel* channel, Client* client, const std::string& target) {
    if (!client->hasCompletedRegistration()) {
        server->SendToClient(client->GetFd(), "451 " + client->getNickname() + " :You have not registered\r\n");
        return false;
    }

    if (target.empty() || (target[0] != '#' && target[0] != '&')) {
        server->SendToClient(client->GetFd(), "403 " + client->getNickname() + " " + target + " :No such channel\r\n");
        return false;
    }

    if (!channel) {
        server->SendToClient(client->GetFd(), "403 " + client->getNickname() + " " + target + " :No such channel\r\n");
        return false;
    }

    if (!channel->hasClient(client)) {
        server->SendToClient(client->GetFd(), "442 " + client->getNickname() + " " + target + " :You're not on that channel\r\n");
        return false;
    }

    if (!channel->isOperator(client)) {
        server->SendToClient(client->GetFd(), "482 " + client->getNickname() + " " + target + " :You're not channel operator\r\n");
        return false;
    }

    return true;
}

static void handleKeyMode(Server* server, Channel* channel, Client* client, bool adding,
                    std::vector<std::string>& paramList, size_t& paramIndex,
                    std::string& modeChanges, std::string& modeParams) {
    if (adding) {
        if (paramIndex < paramList.size()) {
            std::string key = paramList[paramIndex++];
            if (!key.empty()) {
                channel->setKey(key);
                modeChanges += 'k';
                if (!modeParams.empty()) modeParams += " ";
                modeParams += key;
            }
        } else {
            server->SendToClient(client->GetFd(), "461 " + client->getNickname() + " MODE :Not enough parameters\r\n");
        }
    } else {
        channel->setKey("");
        modeChanges += 'k';
    }
}

static void handleLimitMode(Server* server, Channel* channel, Client* client, bool adding,
                    std::vector<std::string>& paramList, size_t& paramIndex,
                    std::string& modeChanges, std::string& modeParams) {
    if (adding) {
        if (paramIndex < paramList.size()) {
            int limit = std::atoi(paramList[paramIndex++].c_str());
            if (limit > 0) {
                channel->setUserLimit(limit);
                modeChanges += 'l';
                if (!modeParams.empty()) modeParams += " ";
                modeParams += paramList[paramIndex - 1];
            }
        } else {
            server->SendToClient(client->GetFd(), "461 " + client->getNickname() + " MODE :Not enough parameters\r\n");
        }
    } else {
        channel->setUserLimit(-1);
        modeChanges += 'l';
    }
}

static void handleOperatorMode(Server* server, Channel* channel, Client* client, bool adding,
                        std::vector<std::string>& paramList, size_t& paramIndex,
                        std::string& modeChanges, std::string& modeParams) {
    if (paramIndex < paramList.size()) {
        std::string targetNick = paramList[paramIndex++];
        Client* targetClient = server->FindClientByNickname(targetNick);

        if (!targetClient || !channel->hasClient(targetClient)) {
            server->SendToClient(client->GetFd(), "441 " + client->getNickname() + " " + targetNick + " " + channel->getName() + " :They aren't on that channel\r\n");
            return;
        }

        if (adding && channel->addOperator(targetClient)) {
            modeChanges += 'o';
            if (!modeParams.empty()) modeParams += " ";
            modeParams += targetNick;
        } else if (!adding && channel->removeOperator(targetClient)) {
            modeChanges += 'o';
            if (!modeParams.empty()) modeParams += " ";
            modeParams += targetNick;
        }
    } else {
        server->SendToClient(client->GetFd(), "461 " + client->getNickname() + " MODE :Not enough parameters\r\n");
    }
}


void Parser::handleMode(Server *server, const std::string &target, const std::string &modes, const std::string &params, int clientFd) {
    Client* client = server->FindClientByFd(clientFd);
    if (!client) return;

    Channel* channel = server->FindChannelByName(target);
    if (!validateModeRequest(server, channel, client, target)) return;

    if (modes.empty()) {
        server->SendToClient(clientFd, "324 " + client->getNickname() + " " + target + " " + channel->getModes() + "\r\n");
        return;
    }

    std::vector<std::string> paramList;
    std::istringstream paramStream(params);
    std::string param;
    while (paramStream >> param) {
        paramList.push_back(param);
    }

    bool adding = true;
    std::string modeChanges;
    std::string modeParams;
    size_t paramIndex = 0;

    for (std::string::size_type i = 0; i < modes.length(); i++) {
        char c = modes[i];
        if (c == '+' || c == '-') {
            adding = (c == '+');
            modeChanges += c;
            continue;
        }

        switch (c) {
            case 'b':
                if (adding) {
                    channel->activateBot();
                    modeChanges += 'b';
                    std::string botMsg = ":BOT!bot@server NOTICE " + target + " :Bot activated in this channel!\r\n";
                    channel->broadcastMessage(botMsg, NULL, server);
                } else {
                    channel->deactivateBot();
                    modeChanges += 'b';
                    std::string botMsg = ":BOT!bot@server NOTICE " + target + " :Bot deactivated in this channel!\r\n";
                    channel->broadcastMessage(botMsg, NULL, server);
                }
                break;
            case 'k':
                handleKeyMode(server, channel, client, adding, paramList, paramIndex, modeChanges, modeParams);
                break;
            case 'l':
                handleLimitMode(server, channel, client, adding, paramList, paramIndex, modeChanges, modeParams);
                break;
            case 'o':
                handleOperatorMode(server, channel, client, adding, paramList, paramIndex, modeChanges, modeParams);
                break;
            case 'i':
            case 't':
            case 'n':
            case 'm':
            case 's':
            case 'p':
                channel->setMode(c, adding);
                modeChanges += c;
                break;
            default:
                server->SendToClient(clientFd, "472 " + client->getNickname() + " " + c + " :is unknown mode char to me\r\n");
        }
    }

    if (modeChanges.length() > 1) {
        std::string modeMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" +
                             client->getIpAdd() + " MODE " + target + " " + modeChanges;
        if (!modeParams.empty()) {
            modeMsg += " " + modeParams;
        }
        modeMsg += "\r\n";
        channel->broadcastMessage(modeMsg, client, server);
    }
}

