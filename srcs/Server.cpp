#include "Server.hpp"
#include "Client.hpp"
#include "Channel.hpp"
#include "CommandHandler.hpp"
#include <iostream>
#include <cstring>
#include <cerrno>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

Server::Server(int port, const std::string& password)
	: _port(port),
	  _password(password),
	  _serverName("ft_irc.42.fr"),
	  _serverSocket(-1),
	  _commandHandler(NULL),
	  _running(false)
{
	std::cout << "Constructing Server object..." << std::endl;
	if (port <= 0 || port > 65535)
		throw std::runtime_error("Invalid port number");
	if (password.empty())
		throw std::runtime_error("Password cannot be empty");
	_setupServerSocket();
	_commandHandler = new CommandHandler(this);
	std::cout << "Server object constructed successfully" << std::endl;
}

Server::~Server()
{
	std::cout << "Destroying Server object..." << std::endl;
	stop();
	
	delete _commandHandler;
	_commandHandler = NULL;
	
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		close(it->first);
		delete it->second;
	}
	_clients.clear();
	
	for (std::map<std::string, Channel*>::iterator it = _channels.begin(); it != _channels.end(); ++it)
		delete it->second;
	_channels.clear();
	

	if (_serverSocket != -1)
	{
		close(_serverSocket);
		std::cout << "Server socket closed" << std::endl;
	}
	std::cout << "Server object destroyed" << std::endl;
}

void Server::_setupServerSocket()
{
	std::cout << "Setting up server socket..." << std::endl;
	
	_serverSocket = socket(AF_INET, SOCK_STREAM, 0);
	if (_serverSocket == -1)
		throw std::runtime_error(std::string("socket() failed: ") + strerror(errno));
	std::cout << "   ✓ Socket created (fd: " << _serverSocket << ")" << std::endl;
	

	int opt = 1;
	if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) == -1)
	{
		close(_serverSocket);
		throw std::runtime_error(std::string("setsockopt() failed: ") + strerror(errno));
	}
	std::cout << "   ✓ Socket options set (SO_REUSEADDR)" << std::endl;
	
	struct sockaddr_in serverAddr;
	std::memset(&serverAddr, 0, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = INADDR_ANY;
	serverAddr.sin_port = htons(_port);

	if (bind(_serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1)
	{
		close(_serverSocket);
		throw std::runtime_error(std::string("bind() failed: ") + strerror(errno));
	}
	std::cout << "   ✓ Socket bound to 0.0.0.0:" << _port << std::endl;
	
	if (listen(_serverSocket, SOMAXCONN) < 0)
	{
		close(_serverSocket);
		throw std::runtime_error(std::string("listen() failed: ") + strerror(errno));
	}
	std::cout << "   ✓ Socket listening (backlog: SOMAXCONN)" << std::endl;
	
	_setNonBlocking(_serverSocket);
	std::cout << "   ✓ Socket set to non-blocking mode" << std::endl;
	
	struct pollfd serverPollFd;
	serverPollFd.fd = _serverSocket;
	serverPollFd.events = POLLIN;
	serverPollFd.revents = 0;
	_pollFds.push_back(serverPollFd);
	std::cout << "   ✓ Server socket added to poll set" << std::endl;
	
	std::cout << "Server socket setup complete" << std::endl;
}

void Server::_setNonBlocking(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	if (flags == -1)
		throw std::runtime_error(std::string("fcntl(F_GETFL) failed: ") + strerror(errno));
	if (fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
		throw std::runtime_error(std::string("fcntl(F_SETFL) failed: ") + strerror(errno));
}

int Server::getPort() const
{
	return (_port);
}

const std::string& Server::getPassword() const
{
	return (_password);
}

const std::string& Server::getServerName() const
{
	return (_serverName);
}

void Server::start()
{
	std::cout << "Starting server event loop..." << std::endl;
	_running = true;
	while (_running)
	{
		int pollCount = poll(&_pollFds[0], _pollFds.size(), -1);
		if (pollCount == -1)
		{
			if (errno == EINTR)
				continue;
			std::cerr << "poll() error: " << strerror(errno) << std::endl;
			break;
		}
		for (size_t i = 0; i < _pollFds.size(); ++i)
		{
			if (_pollFds[i].revents == 0)
				continue;
			if (_pollFds[i].fd == _serverSocket)
			{
				if (_pollFds[i].revents & POLLIN)
					_acceptNewClient();
			}
			else
			{
				int clientFd = _pollFds[i].fd;
				
				if (_pollFds[i].revents & POLLHUP)
				{
					std::cout << "Client " << clientFd << " hung up (POLLHUP)" << std::endl;
					_disconnectClient(clientFd);
					break;
				}
				if (_pollFds[i].revents & POLLERR)
				{
					std::cerr << "Socket error on client " << clientFd << " (POLLERR)" << std::endl;
					_disconnectClient(clientFd);
					break;
				}
				if (_pollFds[i].revents & POLLNVAL)
				{
					std::cerr << "Invalid fd " << clientFd << " (POLLNVAL)" << std::endl;
					_disconnectClient(clientFd);
					break;
				}
				
				if (_pollFds[i].revents & POLLIN)
					_handleClientData(clientFd);
				
				if (_pollFds[i].revents & POLLOUT)
					_sendPendingData(clientFd);
			}
		}
	}
	std::cout << "Server event loop stopped" << std::endl;
}

void Server::stop()
{
	if (_running)
	{
		std::cout << "Stopping server..." << std::endl;
		_running = false;
	}
}

void Server::printStats() const
{
	std::cout << "\nServer Statistics:" << std::endl;
	std::cout << "  Clients connected: " << _clients.size() << std::endl;
	std::cout << "  Channels active: " << _channels.size() << std::endl;
	std::cout << "  Poll fds: " << _pollFds.size() << " (1 server + " 
	          << (_pollFds.size() - 1) << " clients)" << std::endl;
	std::cout << "  Server running: " << (_running ? "Yes" : "No") << std::endl;
}

Client* Server::getClient(int fd)
{
	std::map<int, Client*>::iterator it = _clients.find(fd);
	if (it != _clients.end())
		return (it->second);
	return (NULL);
}


Client* Server::getClientByNick(const std::string& nickname)
{
	for (std::map<int, Client*>::iterator it = _clients.begin(); it != _clients.end(); ++it)
	{
		if (it->second && it->second->getNickname() == nickname)
			return (it->second);
	}
	return (NULL);
}

void Server::removeClient(int fd)
{
	(void)fd;
}

Channel* Server::getChannel(const std::string& name)
{
	std::map<std::string, Channel*>::iterator it = _channels.find(name);
	if (it != _channels.end())
		return (it->second);
	return (NULL);
}

Channel* Server::createChannel(const std::string& name)
{
	Channel* existingChannel = getChannel(name);
	if (existingChannel)
	{
		std::cout << "Channel " << name << " already exists" << std::endl;
		return (existingChannel);
	}

	Channel* newChannel = new Channel(name);
	_channels[name] = newChannel;
	std::cout << "Channel " << name << " created (" 
	          << _channels.size() << " channels active)" << std::endl;
	return (newChannel);
}


void Server::removeChannel(const std::string& name)
{
	std::map<std::string, Channel*>::iterator it = _channels.find(name);
	if (it != _channels.end())
	{
		delete it->second;
		_channels.erase(it);
		std::cout << "Channel " << name << " removed (" 
		          << _channels.size() << " channels remaining)" << std::endl;
	}
	else
		std::cout << "Channel " << name << " not found" << std::endl;
}

void Server::broadcastToChannel(const std::string& channelName, const std::string& message, int excludeFd)
{
	Channel* channel = getChannel(channelName);
	if (!channel)
	{
		std::cerr << "Channel " << channelName << " not found for broadcast" << std::endl;
		return;
	}
	
	std::set<Client*> members = channel->getMembers();
	std::cout << "Broadcasting to channel " << channelName 
	          << " (" << members.size() << " members)" << std::endl;
	
	for (std::set<Client*>::iterator it = members.begin(); it != members.end(); ++it)
	{
		if (*it && (*it)->getFd() != excludeFd)
		{
			(*it)->sendMessage(message);
			_enablePollOut((*it)->getFd());
		}
	}
	
	std::cout << "   ✓ Message broadcasted: " << message.substr(0, 50) 
	          << (message.length() > 50 ? "..." : "") << std::endl;
}

void Server::_acceptNewClient()
{
	struct sockaddr_in clientAddr;
	socklen_t clientAddrLen = sizeof(clientAddr);
	while (true)
	{
		int clientFd = accept(_serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);
		if (clientFd == -1)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				break;
			if (errno == EINTR)
				continue;
			std::cerr << "accept() error: " << strerror(errno) << std::endl;
			break;
		}

		char clientIP[INET_ADDRSTRLEN];
		inet_ntop(AF_INET, &clientAddr.sin_addr, clientIP, INET_ADDRSTRLEN);
		int clientPort = ntohs(clientAddr.sin_port);
		
		std::cout << "New connection from " << clientIP << ":" << clientPort 
		          << " (fd: " << clientFd << ")" << std::endl;
		try
		{
			_setNonBlocking(clientFd);
		}
		catch (const std::exception& e)
		{
			std::cerr << "Failed to set client socket non-blocking: " << e.what() << std::endl;
			close(clientFd);
			continue;
		}
		
		Client* newClient = new Client(clientFd, clientIP, clientPort);
		_clients[clientFd] = newClient;
		
		struct pollfd clientPollFd;
		clientPollFd.fd = clientFd;
		clientPollFd.events = POLLIN;
		clientPollFd.revents = 0;
		_pollFds.push_back(clientPollFd);
		
		std::cout << "Client " << clientFd << " added to poll set (" 
		          << _clients.size() << " clients connected)" << std::endl;
	}
}

void Server::_handleClientData(int fd)
{
	char buffer[4096];
	ssize_t bytesRead;
	
	while (true)
	{
		bytesRead = recv(fd, buffer, sizeof(buffer) - 1, 0);
		
		if (bytesRead > 0)
		{
			buffer[bytesRead] = '\0';
			std::cout << "Received " << bytesRead << " bytes from client " << fd << std::endl;
			
			Client* client = getClient(fd);
			if (client)
			{
				client->appendToRecvBuffer(buffer, bytesRead);
				
				std::string command;
				while (client->extractCommand(command))
				{
					if (_commandHandler)
						_commandHandler->handleCommand(client, command);
				}
			}
		}
		else if (bytesRead == 0)
		{

			std::cout << "Client " << fd << " closed connection" << std::endl;
			_disconnectClient(fd);
			break;
		}
		else
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
				break;
			else if (errno == EINTR)
				continue;
			else
			{
				std::cerr << "recv() error on client " << fd << ": " << strerror(errno) << std::endl;
				_disconnectClient(fd);
				break;
			}
		}
	}
}

void Server::_disconnectClient(int fd)
{
	std::cout << "Disconnecting client " << fd << std::endl;
	
	std::map<int, Client*>::iterator it = _clients.find(fd);
	if (it != _clients.end())
	{
		if (it->second != NULL)
		{
			std::string nickname = it->second->getNickname();
			if (!nickname.empty())
				std::cout << "   Client nickname: " << nickname << std::endl;
			
			const std::set<std::string>& channels = it->second->getChannels();
			for (std::set<std::string>::const_iterator chanIt = channels.begin(); chanIt != channels.end(); ++chanIt)
			{
				Channel* chan = getChannel(*chanIt);
				if (chan)
				{
					std::string quitMsg = it->second->getPrefix() + " QUIT :Client disconnected\r\n";
					broadcastToChannel(*chanIt, quitMsg, fd);
					chan->removeMember(it->second);
					chan->removeOperator(it->second);
				}
			}
			
			delete it->second;
		}
		
		_clients.erase(it);
		std::cout << "   ✓ Client removed from client list" << std::endl;
	}
	else
		std::cout << "   Warning: Client " << fd << " not found in client map" << std::endl;
	
	_removePollFd(fd);
	shutdown(fd, SHUT_RDWR);
	
	if (close(fd) == -1)
		std::cerr << "   close() error: " << strerror(errno) << std::endl;
	else
		std::cout << "   ✓ Socket closed" << std::endl;
	
	std::cout << "Client " << fd << " disconnected (" 
	          << _clients.size() << " clients remaining)" << std::endl;
}

void Server::_sendToClient(int fd, const std::string& message)
{
	ssize_t bytesSent = send(fd, message.c_str(), message.length(), 0);
	if (bytesSent == -1)
	{
		if (errno == EWOULDBLOCK || errno == EAGAIN)
			std::cerr << "Socket buffer full for client " << fd << ", message should be queued" << std::endl;
		else
		{
			std::cerr << "send() error on client " << fd << ": " << strerror(errno) << std::endl;
			_disconnectClient(fd);
		}
	}
	else if (bytesSent < (ssize_t)message.length())
		std::cout << "Partial send: " << bytesSent << "/" << message.length() << " bytes" << std::endl;
	else
		std::cout << "Sent " << bytesSent << " bytes to client " << fd << std::endl;
}

void Server::_sendPendingData(int fd)
{
	Client* client = getClient(fd);
	if (!client)
	{
		std::cerr << "Client " << fd << " not found in _sendPendingData" << std::endl;
		return;
	}
	
	const std::string& sendBuffer = client->getSendBuffer();
	if (sendBuffer.empty())
	{
		_disablePollOut(fd);
		return;
	}
	
	ssize_t bytesSent = send(fd, sendBuffer.c_str(), sendBuffer.length(), 0);
	if (bytesSent > 0)
	{
		std::cout << "Sent " << bytesSent << " bytes to client " << fd << std::endl;
		client->consumeSendBuffer(bytesSent);
		
		if (client->getSendBuffer().empty())
			_disablePollOut(fd);
	}
	else if (bytesSent == -1)
	{
		if (errno != EWOULDBLOCK && errno != EAGAIN)
		{
			std::cerr << "send() error: " << strerror(errno) << std::endl;
			_disconnectClient(fd);
		}
	}
}

void Server::_enablePollOut(int fd)
{
	for (std::vector<struct pollfd>::iterator it = _pollFds.begin(); it != _pollFds.end(); ++it)
	{
		if (it->fd == fd)
		{
			if (!(it->events & POLLOUT))
			{
				it->events |= POLLOUT;
				std::cout << "   ✓ POLLOUT enabled for fd " << fd << std::endl;
			}
			return;
		}
	}
	std::cerr << "Warning: fd " << fd << " not found in _enablePollOut" << std::endl;
}

void Server::_disablePollOut(int fd)
{
	for (std::vector<struct pollfd>::iterator it = _pollFds.begin(); it != _pollFds.end(); ++it)
	{
		if (it->fd == fd)
		{
			if (it->events & POLLOUT)
			{
				it->events &= ~POLLOUT;
				std::cout << "   ✓ POLLOUT disabled for fd " << fd << std::endl;
			}
			return;
		}
	}
	std::cerr << "Warning: fd " << fd << " not found in _disablePollOut" << std::endl;
}

void Server::_removePollFd(int fd)
{
	for (std::vector<struct pollfd>::iterator it = _pollFds.begin(); it != _pollFds.end(); ++it)
	{
		if (it->fd == fd)
		{
			_pollFds.erase(it);
			std::cout << "   ✓ File descriptor " << fd << " removed from poll set" << std::endl;
			return;
		}
	}
	std::cerr << "Warning: fd " << fd << " not found in poll set" << std::endl;
}
