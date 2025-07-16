#include "../../incs/parsing.hpp"
#include "../../incs/server.hpp"
#include "../../incs/client.hpp"
#include "../../incs/channel.hpp"
#include <sstream>

void Parser::handleMode(Server *server, const std::string &target, const std::string &modes, const std::string &params, int clientFd) {
    Client* client = server->FindClientByFd(clientFd);
    if (!client) {
        return;
    }
    
    // Check if client is fully registered
    if (!client->hasCompletedRegistration()) {
        server->SendToClient(clientFd, "451 " + client->getNickname() + " :You have not registered\r\n");
        return;
    }
    
    std::cout << "DEBUG MODE: Client " << client->getNickname() << " executing MODE " << target << " " << modes << " " << params << std::endl;
    
    // Check if target is a channel
    if (target.empty() || (target[0] != '#' && target[0] != '&')) {
        server->SendToClient(clientFd, "403 " + client->getNickname() + " " + target + " :No such channel\r\n");
        return;
    }
    
    // Find channel
    Channel* channel = server->FindChannelByName(target);
    if (!channel) {
        server->SendToClient(clientFd, "403 " + client->getNickname() + " " + target + " :No such channel\r\n");
        return;
    }
    
    std::cout << "DEBUG MODE: Channel found: " << target << std::endl;
    
    // Check if client is in channel
    if (!channel->hasClient(client)) {
        server->SendToClient(clientFd, "442 " + client->getNickname() + " " + target + " :You're not on that channel\r\n");
        return;
    }
    
    std::cout << "DEBUG MODE: Client is in channel" << std::endl;
    
    // If no modes specified, return current modes
    if (modes.empty()) {
        std::string currentModes = channel->getModes();
        server->SendToClient(clientFd, "324 " + client->getNickname() + " " + target + " " + currentModes + "\r\n");
        return;
    }
    
    // Check if client is operator for mode changes
    if (!channel->isOperator(client)) {
        server->SendToClient(clientFd, "482 " + client->getNickname() + " " + target + " :You're not channel operator\r\n");
        return;
    }
    
    std::cout << "DEBUG MODE: Client is operator, proceeding with mode changes" << std::endl;
    
    // Parse parameters
    std::istringstream paramStream(params);
    std::vector<std::string> paramList;
    std::string param;
    while (paramStream >> param) {
        paramList.push_back(param);
    }
    
    std::cout << "DEBUG MODE: Parsed " << paramList.size() << " parameters" << std::endl;
    
    // Process modes
    bool adding = true;
    std::string modeChanges;
    std::string modeParams;
    size_t paramIndex = 0;
    
    std::cout << "DEBUG MODE: Mode string to parse: '" << modes << "'" << std::endl;
    
    for (size_t i = 0; i < modes.length(); i++) {
        char c = modes[i];
        std::cout << "DEBUG MODE: Processing character '" << c << "'" << std::endl;
        
        if (c == '+') {
            adding = true;
            std::cout << "DEBUG MODE: Set to adding mode" << std::endl;
            if (modeChanges.empty() || modeChanges[modeChanges.length() - 1] != '+') {
                modeChanges += '+';
            }
        } else if (c == '-') {
            adding = false;
            std::cout << "DEBUG MODE: Set to removing mode" << std::endl;
            if (modeChanges.empty() || modeChanges[modeChanges.length() - 1] != '-') {
                modeChanges += '-';
            }
        } else {
            // Process mode character
            std::cout << "DEBUG MODE: Processing mode character '" << c << "', adding=" << adding << std::endl;
            
            if (c == 'k') {
                std::cout << "DEBUG MODE: Processing key mode (k), adding=" << adding << std::endl;
                
                if (adding) {
                    // Setting key - need parameter
                    if (paramIndex < paramList.size()) {
                        std::string key = paramList[paramIndex++];
                        std::cout << "DEBUG MODE: Setting key to '" << key << "'" << std::endl;
                        
                        if (!key.empty()) {
                            channel->setKey(key);
                            modeChanges += 'k';
                            if (!modeParams.empty()) modeParams += " ";
                            modeParams += key;
                            std::cout << "DEBUG MODE: Key set successfully" << std::endl;
                        } else {
                            std::cout << "DEBUG MODE: Key is empty, not setting" << std::endl;
                        }
                    } else {
                        std::cout << "DEBUG MODE: No key parameter provided" << std::endl;
                        server->SendToClient(clientFd, "461 " + client->getNickname() + " MODE :Not enough parameters\r\n");
                        return;
                    }
                } else {
                    // Removing key
                    std::cout << "DEBUG MODE: Removing key" << std::endl;
                    channel->setKey("");
                    modeChanges += 'k';
                }
            } else if (c == 'l') {
                if (adding) {
                    // Setting limit - need parameter
                    if (paramIndex < paramList.size()) {
                        int limit = std::atoi(paramList[paramIndex++].c_str());
                        if (limit > 0) {
                            channel->setUserLimit(limit);
                            modeChanges += 'l';
                            if (!modeParams.empty()) modeParams += " ";
                            modeParams += paramList[paramIndex - 1];
                        }
                    } else {
                        server->SendToClient(clientFd, "461 " + client->getNickname() + " MODE :Not enough parameters\r\n");
                        return;
                    }
                } else {
                    // Removing limit
                    channel->setUserLimit(-1);
                    modeChanges += 'l';
                }
            } else if (c == 'i' || c == 't' || c == 'n' || c == 'm' || c == 's' || c == 'p') {
                // Simple modes without parameters
                channel->setMode(c, adding);
                modeChanges += c;
                std::cout << "DEBUG MODE: Set mode '" << c << "' to " << (adding ? "true" : "false") << std::endl;
            } else {
                std::cout << "DEBUG MODE: Unknown mode '" << c << "'" << std::endl;
                server->SendToClient(clientFd, "472 " + client->getNickname() + " " + c + " :is unknown mode char to me\r\n");
            }
        }
    }
    
    std::cout << "DEBUG MODE: Final mode changes string: '" << modeChanges << "'" << std::endl;
    
    // Send mode change confirmation if any changes were made
    if (modeChanges.length() > 1) {  // More than just '+' or '-'
        std::string modeMsg = ":" + client->getNickname() + "!" + client->getUsername() + "@" + client->getIpAdd() + " MODE " + target + " " + modeChanges;
        if (!modeParams.empty()) {
            modeMsg += " " + modeParams;
        }
        modeMsg += "\r\n";
        
        std::cout << "DEBUG MODE: Broadcasting mode message: " << modeMsg.substr(0, modeMsg.length() - 2) << std::endl;
        
        // Send to all clients in channel
        channel->broadcastMessage(modeMsg);
        
        std::cout << "DEBUG MODE: Mode message broadcasted" << std::endl;
    }
    
    // Debug: Show final channel state
    std::cout << "DEBUG MODE: Final channel modes: " << channel->getModes() << std::endl;
    std::cout << "DEBUG MODE: Channel has key: ";
    if (channel->hasKey()) {
        std::cout << "YES" << std::endl;
    } else {
        std::cout << "NO" << std::endl;
    }
}