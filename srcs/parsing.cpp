#include "../incs/ircserv.hpp"
#include <sstream>

void Parser::MainParser(Server *server, char *buffer, int clientFd) {
    std::string buff(buffer);
    std::istringstream iss(buff);
    std::string command;

    if (!buff.empty() && buff[buff.length() - 1] == '\n') {
        buff.erase(buff.length() - 1);
    }
    if (!buff.empty() && buff[buff.length() - 1] == '\r') {
        buff.erase(buff.length() - 1);
    }
    
    iss >> command;
    
    if (command == "PASS") {
        std::string password;
        iss >> password;
        handlePass(server, password, clientFd);
    } 
    else if (command == "NICK") {
        std::string nickname;
        iss >> nickname;
        handleNick(server, nickname, clientFd);
    } 
    else if (command == "USER") {
        std::string username, hostname, servername;
        iss >> username >> hostname >> servername;
        
        std::string realname;
        size_t pos = buff.find(':', 0);
        if (pos != std::string::npos) {
            realname = buff.substr(pos + 1);
        }
        
        handleUser(server, username, realname, clientFd);
    }
    else if (command == "PRIVMSG") {
        std::string target;
        iss >> target;
        
        std::string message;
        size_t pos = buff.find(':', 0);
        if (pos != std::string::npos) {
            message = buff.substr(pos + 1);
        }
        
        handlePrivmsg(server, target, message, clientFd);
    } 
    else if (command == "JOIN") {
        std::string channelName, key;

        iss >> channelName;

        if (channelName.empty()) {
            server->SendToClient(clientFd, "461 * JOIN :Not enough parameters\r\n");
            return;
        }

        if (iss >> key) {
            std::cout << "DEBUG: JOIN with key: channel='" << channelName << "', key='" << key << "'" << std::endl;
        } else {
            key = "";
            std::cout << "DEBUG: JOIN without key: channel='" << channelName << "'" << std::endl;
        }
        
        handleJoin(server, channelName, key, clientFd);
    }
    else if (command == "PART") {
        std::string channelName;
        iss >> channelName;
        
        if (channelName.empty()) {
            server->SendToClient(clientFd, "461 * PART :Not enough parameters\r\n");
            return;
        }
        
        handlePart(server, channelName, clientFd);
    }
    else if (command == "TOPIC") {
        std::string channelName;
        iss >> channelName;
        
        std::string topic;
        size_t pos = buff.find(':', 0);
        if (pos != std::string::npos) {
            topic = buff.substr(pos + 1);
        }
        
        handleTopic(server, channelName, topic, clientFd);
    }
    else if (command == "KICK") {
        std::string channelName, targetNick;
        iss >> channelName >> targetNick;
        
        std::string reason;
        size_t pos = buff.find(':', 0);
        if (pos != std::string::npos) {
            reason = buff.substr(pos + 1);
        }
        
        handleKick(server, channelName, targetNick, reason, clientFd);
    }
    else if (command == "INVITE") {
        std::string targetNick, channelName;
        iss >> targetNick >> channelName;
        handleInvite(server, targetNick, channelName, clientFd);
    }
    else if (command == "MODE") {
        std::string target, modes;
        iss >> target >> modes;

        std::string params;
        std::string word;
        while (iss >> word) {
            if (!params.empty()) {
                params += " ";
            }
            params += word;
        }

        std::cout << "DEBUG MODE PARSING: target='" << target << "', modes='" << modes << "', params='" << params << "'" << std::endl;
        handleMode(server, target, modes, params, clientFd);  // Pass params too
    }
    else {
        server->SendToClient(clientFd, "421 * " + command + " :Unknown command\r\n");
    }
}