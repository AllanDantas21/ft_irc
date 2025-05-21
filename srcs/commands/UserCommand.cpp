#include "../../incs/ircserv.hpp"

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