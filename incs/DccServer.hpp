#pragma once

#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>


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

		int32_t init();
		std::string createDccMessage(const std::string &localIP) const;
		bool handleConnection();
		bool isActive() const;
		int getSockfd() const;

};
