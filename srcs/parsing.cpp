#include "../incs/ircserv.hpp"

void Parser::MainParser(Server *server, char *buffer) {
    std::string buff(buffer);
    std::cout << "Comando Recebido: " << buff;
    server->SendToClient(1, "Comando Recebido: " + buff);
}