#include "../incs/ircserv.hpp"
#include <stdexcept>
#include <string>
#include <algorithm>
#include <cctype>

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
    
    char* endptr;
    long portValue = std::strtol(argv[1], &endptr, 10);
    if (*endptr != '\0' || endptr == argv[1]) {
        throw std::runtime_error("Porta deve ser um número válido.");
    }
    
    this->validatePort(static_cast<int>(portValue));
    SignalConfigurator::configure();
    introduce();
    this->server.ServerInit(static_cast<int>(portValue), argv[2]);
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
    
    std::string password(args[2]);
    if (password.empty() || password.find_first_not_of(" \t\r\n") == std::string::npos) {
        throw std::runtime_error("Senha não pode estar vazia.");
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

bool isControlChar(char c) {
    return c < 32 || c == 127;
}

std::string sanitizeString(const std::string& input) {
    std::string result = input;

    result.erase(std::remove_if(result.begin(), result.end(), isControlChar), result.end());
    
    if (result.length() > 512) {
        result = result.substr(0, 512);
    }
    
    return (result);
}