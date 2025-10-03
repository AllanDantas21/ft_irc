#include "../../incs/ircserv.hpp"
#include <sstream>

static const size_t MAX_NICKNAME_LENGTH = 9;


static bool isValidNicknameStart(char c);
static bool isValidNicknameChar(char c);
static bool isControlChar(char c);

static bool validateNickname(Server *server, const std::string &nickname, int clientFd) {
    if (nickname.length() > MAX_NICKNAME_LENGTH) {
        std::ostringstream oss;
        oss << "432 * " << nickname << " :Nickname too long (max " << MAX_NICKNAME_LENGTH << " chars)\r\n";
        server->SendToClient(clientFd, oss.str());
        return false;
    }
    
    if (!isValidNicknameStart(nickname[0])) {
        server->SendToClient(clientFd, "432 * " + nickname + " :Nickname must start with letter or special char\r\n");
        return false;
    }
    
    for (size_t i = 0; i < nickname.length(); i++) {
        if (!isValidNicknameChar(nickname[i])) {
            if (isControlChar(nickname[i])) {
                server->SendToClient(clientFd, "432 * " + nickname + " :Nickname contains invalid characters\r\n");
            } else {
                server->SendToClient(clientFd, "432 * " + nickname + " :Erroneous nickname\r\n");
            }
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
    
    std::string oldNickname = client->getNickname();
    client->SetNickname(sanitizedNickname);
    
    if (oldNickname.empty()) {
        server->SendToClient(clientFd, "001 " + sanitizedNickname + " :Welcome to the IRC server\r\n");
    } else {
        server->SendToClient(clientFd, ":" + oldNickname + " NICK " + sanitizedNickname + "\r\n");
    }
    
    server->CheckClientAuthentication(client);
}

static bool isValidNicknameStart(char c) {
    return isalpha(c) || c == '_' || c == '[' || c == '{' || 
           c == '\\' || c == '|' || c == '`' || c == '^';
}

static bool isValidNicknameChar(char c) {
    return isalnum(c) || c == '-' || c == '_' || c == '[' || c == ']' || 
           c == '{' || c == '}' || c == '\\' || c == '|' || c == '`' || c == '^';
}

static bool isControlChar(char c) {
    return c < 32 || c == 127;
}
