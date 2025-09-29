#include "../incs/DccClient.hpp"

#include <cstdio>
#include <cstring>
#include <errno.h>
#include <fcntl.h>
#include <ctime>
#include <iomanip>

std::string DccClient::displayTimestamp( void )
{
	time_t now = std::time(NULL);
	struct tm timenow = *std::localtime(&now);

	std::stringstream ss;
	ss << "[" << timenow.tm_year + 1900 <<
		std::setfill('0') << std::setw(2) << timenow.tm_mon + 1 <<
		std::setfill('0') << std::setw(2) << timenow.tm_mday << "_" <<
		std::setfill('0') << std::setw(2) << timenow.tm_hour <<
		std::setfill('0') << std::setw(2) << timenow.tm_min <<
		std::setfill('0') << std::setw(2) << timenow.tm_sec << "]-";

	return ss.str();

}

DccClient::DccClient(const std::string &filename, const std::string &host, int port, size_t filesize, int clientFd)
	: _filename(filename), _host(host), _port(port), _filesize(filesize), _sockfd(-1), _clientFd(clientFd), _isActive(false), _totalBytesReceived(0)
{
	std::string uniqueFilename = displayTimestamp() + _filename;
	_filename = uniqueFilename;
	this->_outputFile.open(_filename.c_str(), std::ios::binary);
	if (!this->_outputFile)
	{
		std::cerr << "Error: creating output file" << std::endl;
		std::stringstream ss;
		ss << "Error: Could not create file '" << _filename << "'\r\n";
		send(clientFd, ss.str().c_str(), ss.str().length(), 0);
	}
}


DccClient::~DccClient() {
	if (this->_outputFile.is_open())
		this->_outputFile.close();
	if (_sockfd != -1)
		close(_sockfd);
}

int DccClient::getSockfd() { return _sockfd; }
bool DccClient::isActive() const { return _isActive; }
int DccClient::getClientFd() { return _clientFd; }

int DccClient::init()
{
	std::cout << "DEBUG: Initializing DCC client for file: " << _filename << std::endl;
	std::cout << "DEBUG: Connecting to " << _host << ":" << _port << std::endl;

	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (_sockfd < 0)
	{
		std::cerr << "Error: creating socket DCC Client: " << strerror(errno) << std::endl;
		return (-1);
	}

	if (fcntl(_sockfd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "Error: setting socket DCC Client non-blocking" << std::endl;
		close(_sockfd);
		_sockfd = -1;
		return (-1);
	}

	struct sockaddr_in serv_addr;
	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(_port);
	serv_addr.sin_addr.s_addr = inet_addr(_host.c_str());

	if (_host.empty() || _port <= 0)
	{
		std::cerr << "Error: Invalid host or port" << std::endl;
		close(_sockfd);
		_sockfd = -1;
		return (-1);
	}

	std::cout << "DEBUG: Attempting connection to " << inet_ntoa(serv_addr.sin_addr) << ":" << ntohs(serv_addr.sin_port) << std::endl;

	if(connect(_sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
	{
		if (errno != EINPROGRESS)
		{
			std::cerr << "Error connecting to DCC server: " << strerror(errno) << std::endl;
			close(_sockfd);
			_sockfd = -1;
			return (-1);
		}
	}
	_isActive = true;
	return (_sockfd);
}

bool DccClient::receiveFile()
{
	char buffer[1024];
	ssize_t bytesReceived = recv(_sockfd, buffer, sizeof(buffer), 0);
	if (bytesReceived <= 0)
	{
		if (bytesReceived == 0)
		{
			_outputFile.close();

			if (_totalBytesReceived == _filesize)
			{
				std::cout << "File received successfully: " << _filename << std::endl;
				std::stringstream ss;
				ss << "File '" << _filename << "' received successfully. Size: " << _totalBytesReceived << " bytes.\r\n";
				send(_clientFd, ss.str().c_str(), ss.str().length(), 0);
			}
			else
			{
				std::cerr << "Error: Connection closed prematurely" << std::endl;
				remove(_filename.c_str());
				std::stringstream ss;
				ss << "Error: File transfer failed - Connection closed prematurely\r\n";
				send(_clientFd, ss.str().c_str(), ss.str().length(), 0);
			}
		}
		else
		{
			std::cerr << "Error: Lost connection or failed to receive file data" << std::endl;
			_outputFile.close();
			remove(_tempFilename.c_str());
			std::stringstream ss;
			ss << "Error: File transfer failed - Lost connection\r\n";
			send(_clientFd, ss.str().c_str(), ss.str().length(), 0);
		}
		_isActive = false;
		return false;
	}
	this->_outputFile.write(buffer, bytesReceived);
	_totalBytesReceived += bytesReceived;
	int progress = (_filesize > 0) ? (static_cast<float>(_totalBytesReceived) / _filesize) * 100 : 100;
	std::stringstream ss;
	ss << "Recebendo " << _filename << ": " << progress << "% completo.\r\n";
	send(_clientFd, ss.str().c_str(), ss.str().length(), 0);

	if (_totalBytesReceived >= _filesize) {
		std::cout << "File received successfully: " << _filename << std::endl;
		_isActive = false;
		return false;
	}

	return true;
}
