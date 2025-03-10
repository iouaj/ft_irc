#include "Server.hpp"

std::vector<Client> Server::_v;
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

void	Server::addChannel(Channel &channel)
{
	Server::_channels.push_back(channel);
}

Channel	*Server::getChannel(std::string name, Client *exec)
{
	std::list<Channel>::iterator it = Server::_channels.begin();

	for (; it != Server::_channels.end(); it++)
	{
		if (!it->getName().compare(name))
			return &*it;
	}
	std::cout << "not found, create new channel" << std::endl;
	return Server::createChannel(name, exec);
}

Channel	*Server::createChannel(std::string name, Client *op)
{
	std::cout << "Creating..." << std::endl;
	Channel	channel(op, name);
	std::cout << "Create" << std::endl;

	Server::_channels.push_back(channel);
	std::cout << "Pushed" << std::endl;

	return Server::getChannel(name, op);
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
