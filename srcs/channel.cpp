#include "../incs/channel.hpp"
#include <algorithm>
#include <sstream>

Channel::Channel(std::string name) : channelName(name), userLimit(-1) {

    modes['i'] = false;
    modes['t'] = true;
    modes['k'] = false;
    modes['l'] = false;
    modes['m'] = false;
    modes['n'] = true;
    modes['s'] = false;
    modes['p'] = false;
}

Channel::~Channel() {
    clientFds.clear();
    operatorFds.clear();
}

bool Channel::isValidName(const std::string& attemptName) {
    if (attemptName.empty() || attemptName.length() > 50) {
        return false;
    }

    if (attemptName[0] != '#' && attemptName[0] != '&') {
        return false;
    }

    for (size_t i = 1; i < attemptName.length(); i++) {
        char c = attemptName[i];
        if (c == ' ' || c == ',' || c == '\0' || c == '\r' || c == '\n' || c == '\7') {
            return false;
        }
    }
    
    return true;
}

std::string Channel::getName() const {
    return channelName;
}

bool Channel::addClient(Client* client) {
    std::cout << "DEBUG: Channel::addClient called for " << (client ? client->getNickname() : "NULL") << " to channel " << channelName << std::endl;
    std::cout << "DEBUG: Current client count: " << clientFds.size() << std::endl;
    
    if (!client || hasClient(client)) {
        std::cout << "DEBUG: addClient failed - client is NULL or already in channel" << std::endl;
        return false;
    }

    if (isBanned(client)) {
        std::cout << "DEBUG: addClient failed - client is banned" << std::endl;
        return false;
    }

    if (isFull()) {
        std::cout << "DEBUG: addClient failed - channel is full" << std::endl;
        return false;
    }

    if (isInviteOnly()) {
        std::string clientMask = client->getNickname() + "!" + client->getUsername() + "@" + client->getIpAdd();
        bool found = false;
        for (size_t i = 0; i < inviteList.size(); i++) {
            if (inviteList[i] == clientMask) {
                found = true;
                break;
            }
        }
        if (!found) {
            return false;
        }
        removeInvite(clientMask);
    }

    clientFds.push_back(client->GetFd());
    
    std::cout << "DEBUG addClient: After push_back, client FD = " << client->GetFd() << std::endl;
    std::cout << "DEBUG addClient: Verifying stored FD..." << std::endl;
    std::cout << "DEBUG addClient: clientFds[" << (clientFds.size()-1) << "] = " << clientFds[clientFds.size()-1] << std::endl;

    if (clientFds.size() == 1) {
        operatorFds.push_back(client->GetFd());
        std::cout << "DEBUG: Client " << client->getNickname() << " is now operator (first in channel)" << std::endl;
    }
    
    std::cout << "DEBUG: addClient successful - channel now has " << clientFds.size() << " clients" << std::endl;
    return true;
}

bool Channel::removeClient(Client* client) {
    if (!client) {
        return false;
    }

    for (size_t i = 0; i < clientFds.size(); i++) {
        if (clientFds[i] == client->GetFd()) {
            clientFds.erase(clientFds.begin() + i);
            break;
        }
    }

    for (size_t i = 0; i < operatorFds.size(); i++) {
        if (operatorFds[i] == client->GetFd()) {
            operatorFds.erase(operatorFds.begin() + i);
            break;
        }
    }
    
    return true;
}

bool Channel::hasClient(Client* client) const {
    if (!client) return false;
    
    for (size_t i = 0; i < clientFds.size(); i++) {
        if (clientFds[i] == client->GetFd()) {
            return true;
        }
    }
    return false;
}

bool Channel::isOperator(Client* client) const {
    if (!client) return false;
    
    for (size_t i = 0; i < operatorFds.size(); i++) {
        if (operatorFds[i] == client->GetFd()) {
            return true;
        }
    }
    return false;
}

bool Channel::addOperator(Client* client) {
    if (!hasClient(client) || isOperator(client)) {
        return false;
    }
    
    operatorFds.push_back(client->GetFd());
    return true;
}

bool Channel::removeOperator(Client* client) {
    for (size_t i = 0; i < operatorFds.size(); i++) {
        if (operatorFds[i] == client->GetFd()) {
            operatorFds.erase(operatorFds.begin() + i);
            return true;
        }
    }
    return false;
}

void Channel::broadcastMessage(const std::string& message, Client* sender, Server* server) {
    (void)server; // Mark as used for compilation
    std::cout << "DEBUG broadcastMessage: Broadcasting to " << clientFds.size() << " clients in channel " << channelName << std::endl;
    std::cout << "DEBUG broadcastMessage: Sender FD = " << (sender ? sender->GetFd() : -1) << std::endl;
    
    for (size_t i = 0; i < clientFds.size(); i++) {
        int clientFd = clientFds[i];
        std::cout << "DEBUG broadcastMessage: Client FD = " << clientFd << std::endl;
        
        // Skip sending to sender
        if (sender && clientFd == sender->GetFd()) {
            std::cout << "DEBUG broadcastMessage: Skipping sender (FD " << clientFd << ")" << std::endl;
            continue;
        }
        
        std::cout << "DEBUG broadcastMessage: Sending to client FD " << clientFd << std::endl;
        ssize_t result = send(clientFd, message.c_str(), message.length(), 0);
        if (result == -1) {
            std::cout << "DEBUG: Error sending message to client " << clientFd << " - " << strerror(errno) << std::endl;
        } else {
            std::cout << "DEBUG broadcastMessage: Successfully sent " << result << " bytes to FD " << clientFd << std::endl;
        }
    }
}

void Channel::setTopic(const std::string& newTopic, Client* setter) {
    if (hasMode('t') && setter != NULL && !isOperator(setter)) {
        return;
    }
    
    topic = newTopic;
    if (setter != NULL) {
        std::string topicMsg = ":" + setter->getNickname() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
        // TODO: Fix this call - needs server parameter
        // broadcastMessage(topicMsg, setter);
    }
}

std::string Channel::getTopic() const {
    return topic;
}

void Channel::setMode(char mode, bool value) {
    modes[mode] = value;

    if (mode == 'k' && !value) {
        key.clear();
    }
    if (mode == 'l' && !value) {
        userLimit = -1;
    }
}

bool Channel::hasMode(char mode) const {
    std::map<char, bool>::const_iterator it = modes.find(mode);
    return it != modes.end() && it->second;
}

std::string Channel::getModes() const {
    std::string modeStr = "+";
    for (std::map<char, bool>::const_iterator it = modes.begin(); it != modes.end(); ++it) {
        if (it->second) {
            modeStr += it->first;
        }
    }
    return modeStr.length() > 1 ? modeStr : "";
}

bool Channel::isInviteOnly() const {
    return hasMode('i');
}

bool Channel::hasKey() const {
    bool result = hasMode('k') && !key.empty();
    std::cout << "DEBUG Channel::hasKey: hasMode('k')=" << hasMode('k') << ", key.empty()=" << key.empty() << ", result=" << result << std::endl;
    return result;
}

bool Channel::checkKey(const std::string& providedKey) const {
    std::cout << "DEBUG checkKey: hasKey=" << hasKey() << ", channelKey='" << key << "', providedKey='" << providedKey << "'" << std::endl;

    if (!hasKey()) {
        return true;
    }

    bool result = (key == providedKey);
    std::cout << "DEBUG checkKey: result=" << (result ? "PASS" : "FAIL") << std::endl;
    return result;
}

void Channel::setKey(const std::string& newKey) {
    std::cout << "DEBUG Channel::setKey: Setting key to '" << newKey << "'" << std::endl;
    key = newKey;
    std::cout << "DEBUG Channel::setKey: Key stored as '" << key << "'" << std::endl;
    setMode('k', !newKey.empty());
    std::cout << "DEBUG Channel::setKey: Mode 'k' set to " << (!newKey.empty() ? "true" : "false") << std::endl;
}

bool Channel::isBanned(Client* client) const {
    if (!client) return false;
    
    std::string clientMask = client->getNickname() + "!" + client->getUsername() + "@" + client->getIpAdd();

    for (size_t i = 0; i < banList.size(); i++) {
        if (banList[i] == clientMask) {
            return true;
        }
    }
    
    return false;
}

void Channel::addBan(const std::string& mask) {
    if (std::find(banList.begin(), banList.end(), mask) == banList.end()) {
        banList.push_back(mask);
    }
}

void Channel::removeBan(const std::string& mask) {
    std::vector<std::string>::iterator it = std::find(banList.begin(), banList.end(), mask);
    if (it != banList.end()) {
        banList.erase(it);
    }
}

void Channel::addInvite(const std::string& mask) {
    if (std::find(inviteList.begin(), inviteList.end(), mask) == inviteList.end()) {
        inviteList.push_back(mask);
    }
}

void Channel::removeInvite(const std::string& mask) {
    std::vector<std::string>::iterator it = std::find(inviteList.begin(), inviteList.end(), mask);
    if (it != inviteList.end()) {
        inviteList.erase(it);
    }
}

void Channel::setUserLimit(int limit) {
    userLimit = limit;
    setMode('l', limit > 0);
}

int Channel::getUserLimit() const {
    return userLimit;
}

bool Channel::isFull() const {
    return userLimit > 0 && static_cast<int>(clientFds.size()) >= userLimit;
}

int Channel::getClientCount() const {
    return clientFds.size();
}

std::vector<Client*> Channel::getClients() const {
    // TODO: Implement this properly - needs server reference
    std::vector<Client*> empty;
    return empty;
}

std::vector<Client*> Channel::getOperators() const {
    // TODO: Implement this properly - needs server reference  
    std::vector<Client*> empty;
    return empty;
}

std::string Channel::getClientsList(Server* server) const {
    std::cout << "DEBUG getClientsList: Channel " << channelName << " has " << clientFds.size() << " clients" << std::endl;
    
    if (clientFds.empty()) {
        std::cout << "DEBUG getClientsList: No clients in channel" << std::endl;
        return "";
    }

    std::stringstream ss;
    for (size_t i = 0; i < clientFds.size(); i++) {
        int clientFd = clientFds[i];
        std::cout << "DEBUG getClientsList: Client FD = " << clientFd << std::endl;
        
        Client* client = server->FindClientByFd(clientFd);
        if (client && !client->getNickname().empty()) {
            if (isOperator(client)) {
                ss << "@";
            }
            ss << client->getNickname();
            if (i < clientFds.size() - 1) {
                ss << " ";
            }
        }
    }
    
    std::string result = ss.str();
    std::cout << "DEBUG getClientsList: Result = '" << result << "'" << std::endl;
    return result;
}

