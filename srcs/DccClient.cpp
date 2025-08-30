#include "../incs/DccClient.hpp"

#include <cstdio>
#include <cstring>
#include <errno.h>

DccClient::DccClient(const std::string &filename, const std::string &host, int port, size_t filesize, int clientFd)
    : _filename(filename), _host(host), _port(port), _filesize(filesize), _sockfd(-1), _clientFd(clientFd), _isActive(false) {}


DccClient::~DccClient() {
	if (_sockfd != -1)
		close(_sockfd);
}

int DccClient::getSockfd() { return _sockfd; }
bool DccClient::isActive() const { return _isActive; }
int DccClient::getClientFd() { return _clientFd; }

int DccClient::init()
{
	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (_sockfd < 0)
	{
		std::cerr << "Error: creating socket DCC Client" << std::endl;
		return (-1);
	}

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(_port);
	serv_addr.sin_addr.s_addr = inet_addr(_host.c_str());

	if(connect(_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		std::cerr << "Error: connecting to DCC server" << std::endl;
		close(_sockfd);
		_sockfd = -1;
		return (-1);
	}
	_isActive = true;
	return (_sockfd);
}

bool DccClient::receiveFile()
{
	std::ofstream outputFile(_filename.c_str(), std::ios::binary);
	if (!outputFile)
	{
		std::cerr << "Error: creating output file" << std::endl;
		_isActive = false;
		return false;
	}

	char buffer[1024];
	size_t totalBytesReceived = 0;
	while (totalBytesReceived < _filesize)
	{
		ssize_t bytesReceived = recv(_sockfd, buffer, sizeof(buffer), 0);
		if (bytesReceived <= 0)
		{
			std::cerr << "Error: Lost connection or failed to receive file data" << std::endl;
			outputFile.close();
			remove(_filename.c_str());
			_isActive = false;
			return false;
		}
		outputFile.write(buffer, bytesReceived);
		totalBytesReceived += bytesReceived;
		int progress = (static_cast<float>(totalBytesReceived) / _filesize) * 100;
		std::stringstream ss;
        ss << "Recebendo " << _filename << ": " << progress << "% completo.\r\n";
        send(_clientFd, ss.str().c_str(), ss.str().length(), 0);
	}
	outputFile.close();
	std::cout << "File received successfully: " << _filename << std::endl;
	_isActive = false;
	return false;
}
