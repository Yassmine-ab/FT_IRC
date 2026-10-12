#include "Client.hpp"
#include <iostream>
#include <sstream>

Client::Client(int fd, const std::string& ipAddress, int port)
	: _fd(fd),
	  _ipAddress(ipAddress),
	  _port(port),
	  _nickname(""),
	  _username(""),
	  _realname(""),
	  _hostname(ipAddress),
	  _authenticated(false),
	  _passwordGiven(false),
	  _registered(false),
	  _recvBuffer(""),
	  _sendBuffer("")
{
	std::cout << "Client object created (fd: " << _fd << ", " << _ipAddress << ":" << _port << ")" << std::endl;
}

Client::~Client()
{
	std::cout << "Client object destroyed (fd: " << _fd;
	if (!_nickname.empty())
		std::cout << ", nickname: " << _nickname;
	std::cout << ")" << std::endl;
}

int Client::getFd() const
{
	return (_fd);
}

std::string Client::getIpAddress() const
{
	return (_ipAddress);
}

int Client::getPort() const
{
	return (_port);
}

std::string Client::getNickname() const
{
	return (_nickname);
}

std::string Client::getUsername() const
{
	return (_username);
}

std::string Client::getRealname() const
{
	return (_realname);
}

std::string Client::getHostname() const
{
	return (_hostname);
}

bool Client::isAuthenticated() const
{
	return (_authenticated);
}

bool Client::isPasswordGiven() const
{
	return (_passwordGiven);
}

bool Client::isRegistered() const
{
	return (_registered);
}

const std::string& Client::getRecvBuffer() const
{
	return (_recvBuffer);
}

const std::string& Client::getSendBuffer() const
{
	return (_sendBuffer);
}

const std::set<std::string>& Client::getChannels() const
{
	return (_channels);
}

void Client::setNickname(const std::string& nickname)
{
	std::cout << "Client " << _fd << " nickname set to: " << nickname << std::endl;
	_nickname = nickname;
}

void Client::setUsername(const std::string& username)
{
	std::cout << "Client " << _fd << " username set to: " << username << std::endl;
	_username = username;
}

void Client::setRealname(const std::string& realname)
{
	_realname = realname;
}

void Client::setHostname(const std::string& hostname)
{
	_hostname = hostname;
}

void Client::setAuthenticated(bool authenticated)
{
	_authenticated = authenticated;
	if (authenticated)
		std::cout << "Client " << _fd << " authenticated" << std::endl;
}

void Client::setPasswordGiven(bool given)
{
	_passwordGiven = given;
}

void Client::setRegistered(bool registered)
{
	_registered = registered;
	if (registered)
		std::cout << "Client " << _fd << " (" << _nickname << ") registered" << std::endl;
}

void Client::appendToRecvBuffer(const char* data, size_t size)
{
	_recvBuffer.append(data, size);
}

bool Client::extractCommand(std::string& command)
{
	size_t pos = _recvBuffer.find("\r\n");
	
	if (pos == std::string::npos)
	{
		pos = _recvBuffer.find('\n');
		if (pos == std::string::npos)
			return (false);
	}
	
	command = _recvBuffer.substr(0, pos);
	
	if (_recvBuffer[pos] == '\r' && pos + 1 < _recvBuffer.length() && _recvBuffer[pos + 1] == '\n')
		_recvBuffer.erase(0, pos + 2);
	else
		_recvBuffer.erase(0, pos + 1);
	
	std::cout << "Command extracted from client " << _fd << ": [" << command << "]" << std::endl;
	return (true);
}

void Client::appendToSendBuffer(const std::string& data)
{
	_sendBuffer.append(data);
	std::cout << "Added " << data.length() << " bytes to send buffer for client " << _fd 
	          << " (total: " << _sendBuffer.length() << " bytes)" << std::endl;
}

void Client::consumeSendBuffer(size_t bytes)
{
	if (bytes >= _sendBuffer.length())
		_sendBuffer.clear();
	else
		_sendBuffer.erase(0, bytes);
}

void Client::clearSendBuffer()
{
	_sendBuffer.clear();
}

void Client::addChannel(const std::string& channelName)
{
	_channels.insert(channelName);
	std::cout << "Client " << _nickname << " joined channel " << channelName 
	          << " (total: " << _channels.size() << " channels)" << std::endl;
}

void Client::removeChannel(const std::string& channelName)
{
	_channels.erase(channelName);
	std::cout << "Client " << _nickname << " left channel " << channelName 
	          << " (remaining: " << _channels.size() << " channels)" << std::endl;
}

bool Client::isInChannel(const std::string& channelName) const
{
	return (_channels.find(channelName) != _channels.end());
}

std::string Client::getPrefix() const
{
	std::stringstream ss;
	ss << ":" << _nickname;
	if (!_username.empty())
		ss << "!" << _username;
	if (!_hostname.empty())
		ss << "@" << _hostname;
	return (ss.str());
}

void Client::sendMessage(const std::string& message)
{
	if (message.length() >= 2 && message.substr(message.length() - 2) == "\r\n")
		appendToSendBuffer(message);
	else
		appendToSendBuffer(message + "\r\n");
}
