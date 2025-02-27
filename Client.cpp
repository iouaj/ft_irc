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

void	Client::setInvisible(void)
{
	this->_visible = false;
}

void	Client::setVisible(void)
{
	this->_visible = true;
}

const bool	&Client::getStatus(void)
{
	const bool &status = this->_visible;
	return status;
}

bool	Client::operator==(const Client &client) const
{
	return this->getFd() == client.getFd() && this->getNickname() == client.getNickname();
}

bool	Client::operator!=(const Client &client) const
{
	return this->getFd() != client.getFd() || this->getNickname() != client.getNickname();
}
