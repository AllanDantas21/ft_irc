#include "incs/server/server.hpp"

int main(){
    try {
        Server server(6667);
        server.start();
    } catch (const std::exception &e) {
        std::cerr << "Erro: " << e.what() << std::endl;
        return (1);
    }
    return (0);
}
