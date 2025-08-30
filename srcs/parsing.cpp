#include "../incs/ircserv.hpp"
#include <sstream>


void Parser::handleDccSend(Server *server, const std::string &target, const std::string &filename, int clientFd)
{
	Client* sender = server->FindClientByFd(clientFd);
	if (!sender)
		return;

	struct stat st;
	if (stat(filename.c_str(), &st) != 0) {
		server->SendToClient(clientFd, "Error: File '" + filename + "' not found\r\n");
		return;
	}
	size_t fileSize = static_cast<size_t>(st.st_size);
	std::string localIP = sender->getIpAdd();
	std::string PortStr = "0";
	DccServer* dccServer = new DccServer(filename, target);
	if (dccServer->init() != -1)
	{
		struct pollfd newPollFd;
		newPollFd.fd = dccServer->getSockfd();
		newPollFd.events = POLLIN;
		server->addPollFd(newPollFd);
		server->addDccServer(dccServer);

		std::stringstream ss;
		ss << "DCC SEND " << filename << " " << localIP << " " << PortStr << " " << fileSize << "\r\n";

		Client* recipient = server->FindClientByNickname(target);
		std::cout << "DEBUG: DCC SEND to target='" << target << "' (nickname='" << (recipient ? recipient->getNickname() : "unknown") << "')" << std::endl;
		if (recipient)
		{
			recipient->setDccMessage(ss.str());
			server->SendToClient(recipient->GetFd(), dccServer->createDccMessage(localIP));
		}
	}
	else
	{
		delete dccServer;
		server->SendToClient(clientFd, "Error: Unable to initiate DCC connection\r\n");
	}
}

void Parser::handleDccGet(Server *server, const std::string &targetNick, int clientFd)
{
	Client* client = server->FindClientByFd(clientFd);
	Client* sender = server->FindClientByNickname(targetNick);
	if (!client || !sender) return;
	std::string dccMessage = sender->getDccMessage();
	if (dccMessage.empty())
	{
		server->SendToClient(clientFd, "Error: No DCC SEND message found for user '" + targetNick + "'\r\n");
		return;
	}
	size_t dccPos = dccMessage.find("DCC SEND ");
	std::string dccParams = dccMessage.substr(dccPos + 9);
	std::string Filename, IpStr, PortStr, SizeStr;
	std::istringstream DccIss(dccParams);
	DccIss >> Filename >> IpStr >> PortStr >> SizeStr;

	DccClient* dccClient = new DccClient(Filename, IpStr, atoi(PortStr.c_str()), static_cast<size_t>(atoi(SizeStr.c_str())), clientFd);
	if (dccClient->init() != -1)
	{
		struct pollfd newPollFd;
		newPollFd.fd = dccClient->getSockfd();
		newPollFd.events = POLLIN;
		server->addPollFd(newPollFd);
		server->addDccClient(dccClient);
		server->SendToClient(clientFd, "DCC GET initiated for file '" + Filename + "' from user '" + targetNick + "'\r\n");
	}
	else
	{
		delete dccClient;
		server->SendToClient(clientFd, "Error: Unable to initiate DCC transfer\r\n");
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
	else if (command == "DCC")
	{
		std::string commandType;
		iss >> commandType;

		if (commandType == "SEND ") {
			std::string target, message;
			iss >> target;
			size_t pos = buff.find(':', 0);
			if (pos != std::string::npos)
				message = buff.substr(pos + 1);
			std::cout << "DEBUG: Detected DCC SEND command" << std::endl;
			handleDccSend(server, message, target, clientFd);
		}
		else if (commandType == "GET") {
			std::string targetNick, filename;
			iss >> targetNick >> filename;
			std::cout << "DEBUG: Detected DCC GET command" << std::endl;
			handleDccGet(server, targetNick, clientFd);
		}
		else
		{
			server->SendToClient(clientFd, "421 * DCC " + commandType + " :Unknown DCC command\r\n");
		}
	}
	else {
		server->SendToClient(clientFd, "421 * " + command + " :Unknown command\r\n");
	}
}
