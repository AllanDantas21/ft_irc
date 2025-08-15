#include "DccServer.hpp"
#include <fstream>
#include <cstring>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>

DccServer::DccServer(const std::string& filename, const std::string& recipient):
	 _sockfd(-1), _filename(filename), _recipient(recipient), _port(0), _isActive(false)
{
	struct stat st;
	if (stat(filename.c_str(), &st) == 0)
		_filesize = st.st_size;
}

DccServer::~DccServer()
{
	if (_sockfd == -1)
		close(_sockfd);
}

bool DccServer::isActive() const { return _isActive; }
int DccServer::getSockfd() const { return _sockfd; }

bool DccServer::init()
{
	// Create socket
	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (_sockfd < 0)
	{
		perror("socket");
		return false;
	}

	//config socket non-blocking
	int flags = fcntl(_sockfd, F_GETFL, 0);
	fcntl(_sockfd, F_SETFL, flags | O_NONBLOCK);

	//config address
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof (addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY; //bind to all interfaces
	addr.sin_port = htons(0); //dinamic port

	//bind
	if (bind(_sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		perror("bind");
		close(_sockfd);
		return false;
	}

	//get port
	struct sockaddr_in bound_addr;
	socklen_t addr_len = sizeof(bound_addr);
	if (getsockname(_sockfd, (struct sockaddr*)&bound_addr, &addr_len) < 0)
	{
		perror("getsockname");
		close(_sockfd);
		return false;
	}

	_port = ntohs(bound_addr.sin_port);
	//init listen
	if (listen(_sockfd, 1) < 0)
	{
		close (_sockfd);
		return false;
	}
	_isActive = true;
	return true;
}

std::string DccServer::createDccMessage(const std::string &localIp) const
{
	//convert IP to number format (required by DCC)
	uint32_t ip_num  = inet_addr(localIp.c_str());

	// Create the DCC message
	return "PRIVMSG " + _recipient + " :\001DCC SEND " + _filename + " " +
			std::to_string(ntohl(ip_num)) + " " +
			std::to_string(_port) + " " +
			std::to_string(_filesize) + "\001\r\n";
}

bool DccServer::handleConnection()
{
	//accept pending connections
	int client_fd = accept(_sockfd, NULL, NULL);
	if (client_fd < 0)
	{
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			return true;
		return false;
	}

	//open file to read in binary mode
	std::ifstream file(_filename, std::ios::binary);
	if (!file)
	{
		close(client_fd);
		return false;
	}

	//read and send file chuncks of 1024 bytes
	char buffer[1024];
	while (!file.eof())
	{
		file.read(buffer, sizeof(buffer));
		std::streamsize bytes_read = file.gcount();
		//send file chunk
		if (send(client_fd, buffer, bytes_read, 0) < 0)
		{
			close(client_fd);
			return false;
		}
	}
	file.close();
	close(client_fd);
	_isActive = false; //connection is done
	return true;
}


