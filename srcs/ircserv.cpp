#include "../incs/ircserv.hpp"

int main(int argc, char **argv)
{
    if (argc != 3){
        std::cerr << "Uso: " << argv[0] << " <porta> <senha>" << std::endl; 
        return (1);
    }
    Server ser;
    introduce();
    try {
        signal(SIGINT, Server::SignalHandler); //-> captura o sinal (ctrl + c)
        signal(SIGQUIT, Server::SignalHandler); //-> captura o sinal (ctrl + \)
        ser.ServerInit(std::atoi(argv[1]), argv[2]); //-> inicializa o servidor
    }
    catch (const std::exception& e){
        ser.CloseFds(); //-> fecha os descritores de arquivo
        std::cerr << e.what() << std::endl;
    }
    std::cout << "O Servidor Foi Fechado!" << std::endl;
}