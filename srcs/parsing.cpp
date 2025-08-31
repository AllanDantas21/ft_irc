#include "../incs/ircserv.hpp"
#include <sstream>


void Parser::handleDccSend(Server *server, const std::string &target, const std::string &filename, int clientFd)
{
	Client* sender = server->FindClientByFd(clientFd);
	if (!sender)
		return;
	std::string cleanFilename = filename;
	if (!cleanFilename.empty() && cleanFilename[cleanFilename.length() - 1] == '\001') {
		cleanFilename.erase(cleanFilename.length() - 1);
	}
	struct stat st;
	if (stat(cleanFilename.c_str(), &st) != 0) {
		server->SendToClient(clientFd, "Error: File '" + cleanFilename + "' not found\r\n");
		return;
	}
	size_t fileSize = static_cast<size_t>(st.st_size);
	std::string localIP = sender->getIpAdd();

	DccServer* dccServer = new DccServer(cleanFilename, target);
	if (dccServer->init() != -1)
	{
		struct pollfd newPollFd;
		newPollFd.fd = dccServer->getSockfd();
		newPollFd.events = POLLIN;
		server->addPollFd(newPollFd);
		server->addDccServer(dccServer);

		// Convert IP to numeric format
		uint32_t ip_num = inet_addr(localIP.c_str());

		std::stringstream ss;
		ss << "\001DCC SEND " << cleanFilename << " " << ntohl(ip_num) << " " << dccServer->getPort() << " " << fileSize << "\001";

		Client* recipient = server->FindClientByNickname(target);
		std::cout << "DEBUG: DCC SEND to target='" << target << "' (nickname='" << (recipient ? recipient->getNickname() : "unknown") << "')" << std::endl;
		std::cout << "DEBUG: DCC parameters - IP: " << localIP << " (" << ntohl(ip_num) << "), Port: " << dccServer->getPort() << ", Size: " << fileSize << std::endl;

		if (recipient)
		{
			recipient->setDccMessage(ss.str());
			std::string msg = ":" + sender->getNickname() + " PRIVMSG " + target + " :" + ss.str() + "\r\n";
			server->SendToClient(recipient->GetFd(), msg);
		}
	}
	else
	{
		delete dccServer;
		server->SendToClient(clientFd, "Error: Unable to initiate DCC connection\r\n");
	}
}

void Parser::handleDccGet(Server *server, const std::string &targetNick, const std::string &, int clientFd)
{
	Client* client = server->FindClientByFd(clientFd);
	Client* sender = server->FindClientByNickname(targetNick);
	if (!client || !sender) return;
	std::string dccMessage = client->getDccMessage();
	if (dccMessage.empty())
	{
		server->SendToClient(clientFd, "Error: No DCC SEND message found for user '" + targetNick + "'\r\n");
		return;
	}
	size_t dccPos = dccMessage.find("DCC SEND ");
	std::string dccParams = dccMessage.substr(dccPos + 9);
	std::string Filename, IpNumStr, PortStr, SizeStr;
	std::istringstream DccIss(dccParams);
	DccIss >> Filename;
	if (!Filename.empty() && Filename[Filename.length() - 1] == '\001') {
		Filename = Filename.substr(0, Filename.length() - 1);
	}
	DccIss >> IpNumStr >> PortStr >> SizeStr;

	// Convert numeric IP back to string format
	uint32_t ip_num;
	std::istringstream(IpNumStr) >> ip_num;
	struct in_addr addr;
	addr.s_addr = htonl(ip_num);
	std::string IpStr = inet_ntoa(addr);

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

		if (message.find("\001DCC SEND ") == 0)
		{
			std::string dccCommand, filename;
			std::istringstream tempStream(message.substr(1)); // Skip the \001
			tempStream >> dccCommand >> dccCommand >> filename; // Read "DCC SEND filename"
			if (!filename.empty() && filename[filename.length() - 1] == '\001') {
				filename = filename.substr(0, filename.length() - 1);
			}
			handleDccSend(server, target, filename, clientFd);
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
	else if (command == "DCC")
	{
		std::string commandType, targetNick, filename, saveAs;
		iss >> commandType >> targetNick >> filename;

		// Get optional save-as filename for GET command
		if (commandType == "GET") {
			iss >> saveAs;
		}

		if (commandType == "SEND") {
			std::cout << "DEBUG: Detected DCC SEND command" << std::endl;
			handleDccSend(server, targetNick, filename, clientFd);
		}
		else if (commandType == "GET") {
			std::cout << "DEBUG: Detected DCC GET command" << std::endl;
			handleDccGet(server, targetNick, saveAs.empty() ? filename : saveAs, clientFd);
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
