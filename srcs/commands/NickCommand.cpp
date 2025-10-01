#include "../../incs/ircserv.hpp"

void Parser::handleNick(Server *server, const std::string &nickname, int clientFd) {
    Client* client = server->FindClientByFd(clientFd);
    if (!client) return;
    
    if (nickname.empty()) {
        server->SendToClient(clientFd, "431 * :No nickname given\r\n");
        return;
    }
    
    std::string sanitizedNickname = sanitizeString(nickname);
    
    if (sanitizedNickname.length() > 9) {
        server->SendToClient(clientFd, "432 * " + sanitizedNickname + " :Nickname too long (max 9 chars)\r\n");
        return;
    }
    
    if (!isalpha(sanitizedNickname[0]) && sanitizedNickname[0] != '_' && sanitizedNickname[0] != '[' && sanitizedNickname[0] != '{' && sanitizedNickname[0] != '\\' && sanitizedNickname[0] != '|' && sanitizedNickname[0] != '`' && sanitizedNickname[0] != '^') {
        server->SendToClient(clientFd, "432 * " + sanitizedNickname + " :Nickname must start with letter or special char\r\n");
        return;
    }
    
    for (size_t i = 0; i < sanitizedNickname.length(); i++) {
        char c = sanitizedNickname[i];
        if (c < 32 || c == 127) {
            server->SendToClient(clientFd, "432 * " + sanitizedNickname + " :Nickname contains invalid characters\r\n");
            return;
        }
        if (!isalnum(c) && c != '-' && c != '_' && c != '[' && c != ']' && c != '{' && c != '}' && c != '\\' && c != '|' && c != '`' && c != '^') {
            server->SendToClient(clientFd, "432 * " + sanitizedNickname + " :Erroneous nickname\r\n");
            return;
        }
    }
    
    if (server->IsNicknameInUse(sanitizedNickname)) {
        server->SendToClient(clientFd, "433 * " + sanitizedNickname + " :Nickname is already in use\r\n");
        return;
    }
    
    client->SetNickname(sanitizedNickname);
    server->SendToClient(clientFd, "Nickname set to " + sanitizedNickname + "\r\n");
    
    server->CheckClientAuthentication(client);
}