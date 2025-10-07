#include "test_helpers.hpp"
#include <algorithm>
#include <cctype>

std::map<int, std::vector<std::string> > MessageCapture::capturedMessages;

static bool isControlCharHelper(char c) {
    return c < 32 || c == 127;
}

std::string sanitizeString(const std::string& input) {
    std::string result = input;
    result.erase(std::remove_if(result.begin(), result.end(), isControlCharHelper), result.end());
    if (result.length() > 512) {
        result = result.substr(0, 512);
    }
    return result;
}

TestServer::TestServer() : server(NULL) {
    MessageCapture::clearAll();
}

TestServer::~TestServer() {
    if (server) {
        delete server;
    }
}

void TestServer::initializeWithPassword(const std::string& password) {
    server = new Server();
    server->SetPort(6667);
    server->SetPassword(password);
}

Client* TestServer::addMockClient(int fd, const std::string& ip) {
    if (!server) {
        return NULL;
    }
    
    Client newClient(fd, ip);
    
    std::vector<Client>& clients = const_cast<std::vector<Client>&>(server->GetClients());
    clients.push_back(newClient);
    
    mockFds.push_back(fd);
    
    return &clients[clients.size() - 1];
}

Client* TestServer::getClientByFd(int fd) {
    if (!server) {
        return NULL;
    }
    return server->FindClientByFd(fd);
}

void TestServer::sendCommand(int clientFd, const std::string& command) {
    if (!server) {
        return;
    }
    
    std::string cmd = command;
    if (cmd[cmd.length() - 1] != '\n') {
        cmd += "\r\n";
    }
    
    char buffer[512];
    std::strncpy(buffer, cmd.c_str(), sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    
    Parser parser;
    parser.MainParser(server, buffer, clientFd);
    
    // Capturar mensagens enfileiradas após o comando
    std::vector<std::string> queuedMessages = server->GetQueuedMessages(clientFd);
    for (size_t i = 0; i < queuedMessages.size(); i++) {
        MessageCapture::capture(clientFd, queuedMessages[i]);
    }
}

std::string TestServer::getLastMessageFor(int fd) {
    return MessageCapture::getLastMessage(fd);
}

void TestServer::clearMessagesFor(int fd) {
    MessageCapture::clear(fd);
}

Server* TestServer::getServer() {
    return server;
}

void MessageCapture::capture(int fd, const std::string& message) {
    capturedMessages[fd].push_back(message);
}

std::vector<std::string> MessageCapture::getMessages(int fd) {
    if (capturedMessages.find(fd) != capturedMessages.end()) {
        return capturedMessages[fd];
    }
    return std::vector<std::string>();
}

std::string MessageCapture::getLastMessage(int fd) {
    std::vector<std::string> messages = getMessages(fd);
    if (messages.empty()) {
        return "";
    }
    return messages[messages.size() - 1];
}

void MessageCapture::clear(int fd) {
    capturedMessages[fd].clear();
}

void MessageCapture::clearAll() {
    capturedMessages.clear();
}

bool MessageCapture::containsMessage(int fd, const std::string& pattern) {
    std::vector<std::string> messages = getMessages(fd);
    for (size_t i = 0; i < messages.size(); i++) {
        if (stringContains(messages[i], pattern)) {
            return true;
        }
    }
    return false;
}

std::string trim(const std::string& str) {
    size_t start = 0;
    size_t end = str.length();
    
    while (start < end && std::isspace(str[start])) {
        start++;
    }
    
    while (end > start && std::isspace(str[end - 1])) {
        end--;
    }
    
    return str.substr(start, end - start);
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::string token;
    std::istringstream tokenStream(str);
    
    while (std::getline(tokenStream, token, delimiter)) {
        tokens.push_back(token);
    }
    
    return tokens;
}

bool stringContains(const std::string& haystack, const std::string& needle) {
    return haystack.find(needle) != std::string::npos;
}