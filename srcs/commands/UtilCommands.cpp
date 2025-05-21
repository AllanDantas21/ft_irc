#include "../../incs/ircserv.hpp"

int Parser::findClientFdFromBuffer(Server *server, const std::string &buffer) {
    (void)buffer; 
    
    for (size_t i = 0; i < server->GetClients().size(); i++) {
        if (server->GetClients()[i].GetFd() > 0) {
            return server->GetClients()[i].GetFd();
        }
    }
    return -1;
}