#pragma once

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>


class DccServer {
	private:
		int _sockfd;
		std::string _filename;
		uint32_t _filesize;
		std::string _recipient;
		uint16_t _port;
		bool _isActive;

	public:
		DccServer(const std::string& filename, const std::string& recipient);
		~DccServer();

		bool init();
		uint16_t getPort() const;
		std::string createDccMessage(const std::string &localIP) const;
		bool handleConnection();
		bool isActive() const;
		int getSockfd() const;

};
