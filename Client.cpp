#include "Client.hpp"

Client::Client(const std::string &nickname, const int fd) : _nickname(nickname), _socket_fd(fd), _invalid_request(0)
{
	this->_visible = true;
	this->_setup = false;
	this->_pass = false;
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

bool	Client::isVisible(void) const
{
	return this->_visible == true;
}

void	Client::setup(void)
{
	this->_setup = true;
	sendServer(*this, RPL_WELCOME(this->getNickname()));
	std::cout << this->_nickname << "!" << this->_username << "@localhost registered" << std::endl;
}

bool	Client::isSetup(void) const
{
	return this->_setup == true;
}

void	Client::pass(void)
{
	this->_pass = true;
}

bool	Client::isPass(void) const
{
	return this->_pass == true;
}

void	Client::incrInvalidRequest(void)
{
	this->_invalid_request += 1;
}

const int	&Client::getInvalidRequest(void) const
{
	const int	&ref = this->_invalid_request;
	return ref;
}

void	Client::sendEverywhere(const std::string &message) const
{
	std::list<Channel>	channels = Server::getAllChannels();
	std::list<Channel>::iterator	it = channels.begin();

	for (; it != channels.end(); it++)
	{
		if (it->haveClient(*this))
			it->broadcast(message + "\r\n");
	}
}

void	Client::sendMessage(const Client &target, const std::string &message) const
{
	std::string request = ":" + this->getNickname() + " PRIVMSG " + target.getNickname() + " :" + message + "\r\n";
	send(target.getFd(), request.c_str(), request.size(), 0);
}

bool	Client::operator==(const Client &client) const
{
	return this->getFd() == client.getFd() && this->getNickname() == client.getNickname();
}

bool	Client::operator!=(const Client &client) const
{
	return this->getFd() != client.getFd() || this->getNickname() != client.getNickname();
}
