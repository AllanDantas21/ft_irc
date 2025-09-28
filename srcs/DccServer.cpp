#include "../incs/DccServer.hpp"
#include <fstream>
#include <cstring>
#include <errno.h>
#include <sstream>


DccServer::DccServer(const std::string& filename, const std::string& recipient):
	 _sockfd(-1), _filename(filename), _filesize(0), _recipient(recipient), _port(0), _isActive(false)
{
	struct stat st;
	if (stat(filename.c_str(), &st) == 0)
	{
		_filesize = st.st_size;
		std::cout << "DEBUG: DccServer found file '" << filename << "'. Size: " << _filesize << " bytes." << std::endl;

	}
	else
	{
		std::cout << "DEBUG: DccServer failed to find file '" << filename << "'. Error: " << strerror(errno) << std::endl;
	}
}

DccServer::~DccServer()
{
	if (_sockfd != -1)
		close(_sockfd);
}

bool DccServer::isActive() const { return _isActive; }
int DccServer::getSockfd() const { return _sockfd; }
uint16_t DccServer::getPort() const { return _port; }

int DccServer::init()
{
	// Create socket
	_sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (_sockfd < 0)
	{
		std::cerr << "Error: creating socket DCC" << std::endl;
		return (-1);
	}

	//config socket non-blocking
	if (fcntl(_sockfd, F_SETFL, O_NONBLOCK) < 0)
	{
		std::cerr << "Error: setting socket DCC non-blocking" << std::endl;
		close(_sockfd);
		return (-1);
	}

	//config address
	struct sockaddr_in addr;
	memset(&addr, 0, sizeof (addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY; //bind to all interfaces
	addr.sin_port = htons(0); //dinamic port

	//bind
	if (bind(_sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
	{
		std::cerr << "Error: binding socket DCC" << std::endl;
		close(_sockfd);
		return (-1);
	}

	//get port
	socklen_t addr_len = sizeof(addr);
	if (getsockname(_sockfd, (struct sockaddr*)&addr, &addr_len) < 0)
	{
		std::cerr << "Error: getting socket name DCC" << std::endl;
		close(_sockfd);
		return (-1);
	}

	_port = ntohs(addr.sin_port);
	//init listen
	if (listen(_sockfd, 1) < 0)
	{
		std::cerr << "Error: listening socket DCC" << std::endl;
		close (_sockfd);
		return (-1);
	}
	_isActive = true;
	return (_sockfd);
}

std::string DccServer::createDccMessage(const std::string &localIp) const
{
	//convert IP to number format (required by DCC)
	uint32_t ip_num  = inet_addr(localIp.c_str());

	std::stringstream ss;
	ss << ntohl(ip_num) << " " << _port << " " << _filesize;
	return ("PRIVMSG " + _recipient + " :DCC SEND " + _filename + " " + ss.str() + "\r\n");
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

	std::cout << "DCC connection established with " << _recipient << std::endl;
	std::cout << "Sending file: " << _filename << " (" << _filesize << " bytes)" << std::endl;

	//open file to read in binary mode
	std::ifstream file(_filename.c_str(), std::ios::binary);
	if (!file)
	{
		close(client_fd);
		_isActive = false;
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
			file.close();
			close(client_fd);
			_isActive = false;
			return false;
		}
	}
	file.close();
	close(client_fd);
	_isActive = false; //connection is done
	std::cout << "File " << _filename << " sent successfully." << std::endl;
	return false;
}



