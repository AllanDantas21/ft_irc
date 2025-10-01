#include "../../incs/ircserv.hpp"

void Parser::handleUser(Server *server, const std::string &username, const std::string &realname, int clientFd) {
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
    
    std::string sanitizedUsername = sanitizeString(username);
    std::string sanitizedRealname = sanitizeString(realname);
    
    if (sanitizedUsername.length() > 10) {
        server->SendToClient(clientFd, "461 * USER :Username too long (max 10 chars)\r\n");
        return;
    }
    
    client->SetUsername(sanitizedUsername);
    client->SetRealname(sanitizedRealname);
    server->SendToClient(clientFd, "Username set to " + sanitizedUsername + "\r\n");
    
    server->CheckClientAuthentication(client);
}