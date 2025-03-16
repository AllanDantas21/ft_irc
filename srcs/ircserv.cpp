#include "../incs/ircserv.hpp"
#include <stdexcept>
#include <string>

int main(int argc, char **argv) {
    ServerController controller;
    controller.execute(argc, argv);
    return (0);
}

void ServerController::execute(int argc, char** argv) {
    try {
        this->prepareExecution(argc, argv);
    } catch (const std::exception& e) {
        return(this->handleError(e));
    }
    this->announceShutdown();
}

void ServerController::prepareExecution(int argc, char** argv) {
    ArgumentValidator::validate(argc, argv);
    SignalConfigurator::configure();
    introduce();
    this->server.ServerInit(std::atoi(argv[1]), argv[2]);
}

void ServerController::validatePort(int portValue) {
    if (portValue < 1024 || portValue > 49151) {
        throw std::runtime_error("Porta inválida. Portas válidas são de 1024 a 49151.");
    }
}

void ArgumentValidator::validate(int count, char** args) {
    if (count != 3) {
        throw std::runtime_error("Uso: " + std::string(args[0]) + " <porta> <senha>");
    }
}

void SignalConfigurator::configure() {
    signal(SIGINT, Server::SignalHandler);
    signal(SIGQUIT, Server::SignalHandler);
}

void ServerController::handleError(const std::exception& e) {
    this->server.CloseFds();
    std::cerr << "Erro: " << e.what() << std::endl;
}

void ServerController::announceShutdown() {
    std::cout << "O Servidor Foi Fechado!" << std::endl;
}