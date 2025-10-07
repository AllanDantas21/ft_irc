#ifndef TEST_HELPERS_HPP
#define TEST_HELPERS_HPP

#include "../../../incs/ircserv.hpp"
#include "../../../incs/server.hpp"
#include "../../../incs/client.hpp"
#include "../../../incs/parsing.hpp"
#include <string>
#include <sstream>
#include <vector>

class TestServer {
private:
    Server* server;
    std::vector<int> mockFds;
    std::map<int, std::string> receivedMessages;

public:
    TestServer();
    ~TestServer();

    void initializeWithPassword(const std::string& password);
    Client* addMockClient(int fd, const std::string& ip = "127.0.0.1");
    Client* getClientByFd(int fd);
    
    void sendCommand(int clientFd, const std::string& command);
    std::string getLastMessageFor(int fd);
    void clearMessagesFor(int fd);
    
    Server* getServer();
};

class MessageCapture {
private:
    static std::map<int, std::vector<std::string> > capturedMessages;
    
public:
    static void capture(int fd, const std::string& message);
    static std::vector<std::string> getMessages(int fd);
    static std::string getLastMessage(int fd);
    static void clear(int fd);
    static void clearAll();
    static bool containsMessage(int fd, const std::string& pattern);
};

std::string trim(const std::string& str);
std::vector<std::string> split(const std::string& str, char delimiter);
bool stringContains(const std::string& haystack, const std::string& needle);

#endif

