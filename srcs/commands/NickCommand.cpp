#include "../../incs/ircserv.hpp"

void Parser::handleNick(Server *server, const std::string &originalMessage, const std::string &nickname, int clientFd) {
    (void)originalMessage;
    
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