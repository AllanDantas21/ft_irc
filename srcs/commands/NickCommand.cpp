#include "../../incs/ircserv.hpp"

static bool validateNickname(Server *server, const std::string &nickname, int clientFd) {
    if (nickname.length() > 9) {
        server->SendToClient(clientFd, "432 * " + nickname + " :Nickname too long (max 9 chars)\r\n");
        return false;
    }
    
    if (!isalpha(nickname[0]) && nickname[0] != '_' && nickname[0] != '[' && nickname[0] != '{' && nickname[0] != '\\' && nickname[0] != '|' && nickname[0] != '`' && nickname[0] != '^') {
        server->SendToClient(clientFd, "432 * " + nickname + " :Nickname must start with letter or special char\r\n");
        return false;
    }
    
    for (size_t i = 0; i < nickname.length(); i++) {
        char c = nickname[i];
        if (c < 32 || c == 127) {
            server->SendToClient(clientFd, "432 * " + nickname + " :Nickname contains invalid characters\r\n");
            return false;
        }
        if (!isalnum(c) && c != '-' && c != '_' && c != '[' && c != ']' && c != '{' && c != '}' && c != '\\' && c != '|' && c != '`' && c != '^') {
            server->SendToClient(clientFd, "432 * " + nickname + " :Erroneous nickname\r\n");
            return false;
        }
    }
    
    return true;
}

void Parser::handleNick(Server *server, const std::string &nickname, int clientFd) {
    Client* client = server->FindClientByFd(clientFd);
    if (!client) return;
    
    if (nickname.empty()) {
        server->SendToClient(clientFd, "431 * :No nickname given\r\n");
        return;
    }
    
    std::string sanitizedNickname = sanitizeString(nickname);
    
    if (!validateNickname(server, sanitizedNickname, clientFd)) {
        return;
    }
    
    if (server->IsNicknameInUse(sanitizedNickname)) {
        server->SendToClient(clientFd, "433 * " + sanitizedNickname + " :Nickname is already in use\r\n");
        return;
    }
    
    client->SetNickname(sanitizedNickname);
    server->SendToClient(clientFd, "Nickname set to " + sanitizedNickname + "\r\n");
    
    server->CheckClientAuthentication(client);
}