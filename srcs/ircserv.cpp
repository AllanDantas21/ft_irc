#include "../incs/ircserv.hpp"

int main(int argc, char **argv) {
    Server ser;
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <porta> <senha>" << std::endl; 
        return 1;
    }
    introduce();
    try {
        signal(SIGINT, Server::SignalHandler);
        signal(SIGQUIT, Server::SignalHandler);
        
        int port = std::atoi(argv[1]);
        if (port < 1024 || port > 65535) {
            throw std::runtime_error("Porta inválida: deve estar entre 1024 e 65535");
        }

        ser.ServerInit(port, argv[2]);
    } catch (const std::exception& e) {
        ser.CloseFds();
        std::cerr << "Erro: " << e.what() << std::endl;
    }
    std::cout << "O Servidor Foi Fechado!" << std::endl;
    return 0;
}