#include "incs/server/server.hpp"
#include "incs/utils/prompt.hpp"

int main()
{
    Server ser;
    introduce();
    try{
        signal(SIGINT, Server::SignalHandler); //-> captura o sinal (ctrl + c)
        signal(SIGQUIT, Server::SignalHandler); //-> captura o sinal (ctrl + \)
        ser.ServerInit(); //-> inicializa o servidor
    }
    catch(const std::exception& e){
        ser.CloseFds(); //-> fecha os descritores de arquivo
        std::cerr << e.what() << std::endl;
    }
    std::cout << "O Servidor Foi Fechado!" << std::endl;
}