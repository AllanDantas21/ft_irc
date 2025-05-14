#include "../incs/ircserv.hpp"
#include <sstream>

void Parser::MainParser(Server *server, char *buffer) {
    std::string buff(buffer);
    std::string command;
    std::istringstream iss(buff);
    
    iss >> command;
    
    if (command == "PASS") {
        std::string password;
        iss >> password;
        handlePass(server, iss.tellg() > 0 ? buff.substr(iss.tellg()) : "", password);
    } 
    else if (command == "NICK") {
        std::string nickname;
        iss >> nickname;
        handleNick(server, buff, nickname);
    } 
    else if (command == "USER") {
        std::string username, hostname, servername, realname;
        iss >> username >> hostname >> servername;
        
        size_t pos = buff.find(':', 0);
        if (pos != std::string::npos) {
            realname = buff.substr(pos + 1);
        }
        
        handleUser(server, buff, username, realname);
    } 
    else {
        int clientFd = findClientFdFromBuffer(server, buff);
        if (clientFd != -1) {
            server->SendToClient(clientFd, "Command not implemented yet: " + buff);
        }
    }
}

void Parser::handlePass(Server *server, const std::string &originalMessage, const std::string &password) {
    int clientFd = findClientFdFromBuffer(server, originalMessage);
    if (clientFd == -1) return;
    
    Client* client = server->FindClientByFd(clientFd);
    if (!client) return;
    
    if (password.empty()) {
        server->SendToClient(clientFd, "461 * PASS :Not enough parameters\r\n");
        return;
    }
    
    std::string cleanPassword = password;
    cleanPassword.erase(0, cleanPassword.find_first_not_of(" \t\r\n"));
    cleanPassword.erase(cleanPassword.find_last_not_of(" \t\r\n") + 1);
    
    if (server->CheckPassword(cleanPassword)) {
        client->SetPasswordValid(true);
        server->SendToClient(clientFd, "Password accepted. Now use NICK and USER to complete registration.\r\n");
    } else {
        server->SendToClient(clientFd, "464 * :Password incorrect\r\n");
    }
}

void Parser::handleNick(Server *server, const std::string &originalMessage, const std::string &nickname) {
    int clientFd = findClientFdFromBuffer(server, originalMessage);
    if (clientFd == -1) return;
    
    Client* client = server->FindClientByFd(clientFd);
    if (!client) return;
    
    if (nickname.empty()) {
        server->SendToClient(clientFd, "431 * :No nickname given\r\n");
        return;
    }
    
    for (size_t i = 0; i < nickname.length(); i++) {
        char c = nickname[i];
        if (!isalnum(c) && c != '-' && c != '_' && c != '[' && c != ']' && c != '{' && c != '}' && c != '\\' && c != '|' && c != '`' && c != '^') {
            server->SendToClient(clientFd, "432 * " + nickname + " :Erroneous nickname\r\n");
            return;
        }
    }
    
    if (server->IsNicknameInUse(nickname)) {
        server->SendToClient(clientFd, "433 * " + nickname + " :Nickname is already in use\r\n");
        return;
    }
    
    client->SetNickname(nickname);
    server->SendToClient(clientFd, "Nickname set to " + nickname + "\r\n");
    
    server->CheckClientAuthentication(client);
}

void Parser::handleUser(Server *server, const std::string &originalMessage, const std::string &username, const std::string &realname) {
    int clientFd = findClientFdFromBuffer(server, originalMessage);
    if (clientFd == -1) return;
    
    Client* client = server->FindClientByFd(clientFd);
    if (!client) return;
    
    if (username.empty() || realname.empty()) {
        server->SendToClient(clientFd, "461 * USER :Not enough parameters\r\n");
        return;
    }
    
    if (!client->getUsername().empty()) {
        server->SendToClient(clientFd, "462 :You may not reregister\r\n");
        return;
    }
    
    client->SetUsername(username);
    client->SetRealname(realname);
    server->SendToClient(clientFd, "Username set to " + username + "\r\n");
    
    server->CheckClientAuthentication(client);
}

int Parser::findClientFdFromBuffer(Server *server, const std::string &buffer) {
    (void)buffer; 
    
    for (size_t i = 0; i < server->GetClients().size(); i++) {
        if (server->GetClients()[i].GetFd() > 0) {
            return server->GetClients()[i].GetFd();
        }
    }
    return -1;
}