#include "ft_irc.hpp"

Client::Client(const std::string &nickname, const int fd) : _nickname(nickname), _socket_fd(fd)
{

}

Client::~Client(void)
{}

void Client::setNickname(const std::string &nickname)
{
	this->_nickname = nickname;
}

const std::string &Client::getNickname(void) const
{
	const std::string	&nick = this->_nickname;
	return nick;
}

const	int	&Client::getFd(void) const
{
	const	int &fd = this->_socket_fd;
	return fd;
}

void	Client::setUsername(const std::string &username)
{
	this->_username = username;
}

const std::string	&Client::getUsername(void) const
{
	const std::string &username = this->_username;
	return username;
}

bool	Client::operator==(const Client &client) const
{
	return this->getFd() == client.getFd() && this->getUsername() == client.getUsername();
}
