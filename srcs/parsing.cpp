#include "../incs/ircserv.hpp"
#include <sstream>


void Parser::handleDccSend(Server *server, const std::string &message, const std::string &target, int clientFd)
{
	std::string Filename, IpStr, PortStr, SizeStr;
	std::istringstream DccIss(message.substr(11));
	DccIss >> Filename >> IpStr >> PortStr >> SizeStr;

	Client* sender = server->FindClientByFd(clientFd);
	std::cout << "DEBUG: DCC SEND command parsed: Filename='" << Filename << "', IP='" << IpStr << "', Port='" << PortStr << "', Size='" << SizeStr << "'" << std::endl;
	std::cout << "DEBUG: DCC SEND from clientFd=" << clientFd << " (nickname='" << (sender ? sender->getNickname() : "unknown") << "')" << std::endl;
	if (!sender)
		return;

	DccServer* dccServer = new DccServer(Filename, target);
	if (dccServer->init() < -1)
	{
		struct pollfd newPollFd;
		newPollFd.fd = dccServer->getSockfd();
		newPollFd.events = POLLIN;
		server->addPollFd(newPollFd);
		server->addDccServer(dccServer);

		std::string localIP = sender->getIpAdd();
		std::string dccMessage = dccServer->createDccMessage(localIP);
		Client* recipient = server->FindClientByNickname(target);
		std::cout << "DEBUG: DCC SEND to target='" << target << "' (nickname='" << (recipient ? recipient->getNickname() : "unknown") << "')" << std::endl;
		if (recipient)
		{
			server->SendToClient(recipient->GetFd(), dccMessage);
		}
	}
	else
	{
		delete dccServer;
		server->SendToClient(clientFd, "Error: Unable to initiate DCC connection\r\n");
	}
}

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

		if (message.length() > 11 && message.substr(0,11) == "\00DCC SEND ")
		{
			handleDccSend(server, message, target, clientFd);
		}
		else
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
