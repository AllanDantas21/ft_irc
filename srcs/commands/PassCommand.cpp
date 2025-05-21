#include "../../incs/ircserv.hpp"

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