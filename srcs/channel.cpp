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
    clients.clear();
    operators.clear();
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
    if (!client || hasClient(client)) {
        return false;
    }

    if (isBanned(client)) {
        return false;
    }

    if (isFull()) {
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

    clients.push_back(client);

    if (clients.size() == 1) {
        operators.push_back(client);
    }
    
    return true;
}

bool Channel::removeClient(Client* client) {
    if (!client) {
        return false;
    }

    std::vector<Client*>::iterator it = std::find(clients.begin(), clients.end(), client);
    if (it != clients.end()) {
        clients.erase(it);
    }

    std::vector<Client*>::iterator opIt = std::find(operators.begin(), operators.end(), client);
    if (opIt != operators.end()) {
        operators.erase(opIt);
    }
    
    return true;
}

bool Channel::hasClient(Client* client) const {
    return std::find(clients.begin(), clients.end(), client) != clients.end();
}

bool Channel::isOperator(Client* client) const {
    return std::find(operators.begin(), operators.end(), client) != operators.end();
}

bool Channel::addOperator(Client* client) {
    if (!hasClient(client) || isOperator(client)) {
        return false;
    }
    
    operators.push_back(client);
    return true;
}

bool Channel::removeOperator(Client* client) {
    std::vector<Client*>::iterator it = std::find(operators.begin(), operators.end(), client);
    if (it != operators.end()) {
        operators.erase(it);
        return true;
    }
    return false;
}

void Channel::broadcastMessage(const std::string& message, Client* sender) {
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i] != sender) {
            send(clients[i]->GetFd(), message.c_str(), message.length(), 0);
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
        broadcastMessage(topicMsg, setter);
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
    return userLimit > 0 && static_cast<int>(clients.size()) >= userLimit;
}

int Channel::getClientCount() const {
    return clients.size();
}

std::vector<Client*> Channel::getClients() const {
    return clients;
}

std::vector<Client*> Channel::getOperators() const {
    return operators;
}

std::string Channel::getClientsList() const {
    if (clients.empty()) {
        return "";
    }

    std::stringstream ss;
    for (size_t i = 0; i < clients.size(); i++) {
        if (clients[i] && !clients[i]->getNickname().empty()) {
            if (isOperator(clients[i])) {
                ss << "@";
            }
            ss << clients[i]->getNickname();
            if (i < clients.size() - 1) {
                ss << " ";
            }
        }
    }
    return ss.str();
}

