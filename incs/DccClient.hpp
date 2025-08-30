#pragma once

#include <string>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <unistd.h>
#include <cstdlib>


class DccClient
{
	private:
		std::string _filename;
		std::string _host;
		int _port;
		size_t _filesize;
		int _sockfd;
		int _clientFd;
		bool _isActive;

	public:
		DccClient(const std::string &filename, const std::string &host, int port, size_t filesize, int c);
		~DccClient();

		int init();
		bool receiveFile();
		int getSockfd();
		int getClientFd();
		bool isActive() const;
};



