#include "../incs/ircserv.hpp"

int main(int argc, char **argv) {
    malloc(40); // TESTE DO VALGRIND
    Server ser;
    if (argc != 3) {
        std::cerr << "Uso: " << argv[0] << " <porta> <senha>" << std::endl; 
        return 1;
    }
    introduce();
    try {
        signal(SIGINT, Server::SignalHandler);
        signal(SIGQUIT, Server::SignalHandler);
        if (std::atoi(argv[1]) < 1024) {
            throw std::runtime_error("Porta inválida");
        }
        ser.ServerInit(std::atoi(argv[1]), argv[2]);
    } catch (const std::exception& e) {
        ser.CloseFds();
        std::cerr << e.what() << std::endl;
    }
    std::cout << "O Servidor Foi Fechado!" << std::endl;
    return 0;
}