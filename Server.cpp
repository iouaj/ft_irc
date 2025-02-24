#include "ft_irc.hpp"
#include <algorithm>

std::vector<Client> Server::_v;
// Channel&	Server::default_channel;
std::list<Channel> Server::_channels;

void	Server::addClient(Client &client)
{
	Server::_v.push_back(client);
}

Client	&Server::getClient(int fd)
{
	std::vector<Client>::iterator	it = Server::_v.begin();

	for (; it != Server::_v.end(); it++)
	{
		if (it->getFd() == fd)
			return *it;
	}
	throw InvalidClientException();
}

Client	&Server::getClient(std::string nickname)
{
	std::vector<Client>::iterator	it = Server::_v.begin();

	for (; it != Server::_v.end(); it++)
	{
		if (!it->getNickname().compare(nickname))
			return *it;
	}
	throw InvalidClientException();
}

// const Channel &Server::getDefaultChannel(void)
// {
// 	const Channel &ref = Server::default_channel;

// 	return ref;
// }

// void	Server::setDefaultChannel(Channel &channel)
// {
// 	Server::default_channel = channel;
// }

void	Server::addChannel(Channel &channel)
{
	Server::_channels.push_back(channel);
}

Channel	&Server::getChannel(std::string name)
{
	std::list<Channel>::iterator it = Server::_channels.begin();

	for (; it != Server::_channels.end(); it++)
	{
		if (!it->getName().compare(name))
			return *it;
	}
	std::cout << "not found, create new channel" << std::endl;
	return Server::createChannel(name);
}

Channel	&Server::createChannel(std::string name)
{
	Channel	channel(name);

	Server::_channels.push_back(channel);

	return Server::getChannel(name);
}

void	Server::deleteChannel(Channel &channel)
{
	Server::_channels.remove(channel);
}

bool	Server::isChannelExist(std::string name)
{
	std::list<Channel>::iterator it = Server::_channels.begin();

	for (; it != Server::_channels.end(); it++)
	{
		if (!it->getName().compare(name))
			return true;
	}
	return false;
}

bool	Server::isClientExist(std::string name)
{
	std::vector<Client>::iterator	it = Server::_v.begin();

	for (; it != Server::_v.end(); it++)
	{
		if (!it->getNickname().compare(name))
			return true;
	}
	return false;
}

const char	*Server::InvalidClientException::what(void) const throw()
{
	return ("Invalid Client");
}
