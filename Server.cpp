#include "Server.hpp"

std::vector<Client> Server::_v;
std::list<Channel> Server::_channels;
std::string Server::_password;
int	Server::_server_socket;
int	Server::_epoll_fd;
std::list<int>	Server::_fds;

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
	if (Server::isChannelExist(channel.getName()) == false)
		Server::_channels.push_back(channel);
}

Channel	*Server::getChannel(std::string name, const Client &exec)
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

Channel	*Server::createChannel(std::string &name, const Client &op)
{
	Channel	channel(op, name);

	Server::_channels.push_back(channel);

	return &Server::_channels.back();
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

void	Server::clearClient(Client &client, const std::string &leave_msg)
{
	std::list<Channel>::iterator	it = Server::_channels.begin();
	const std::string msg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost QUIT :" + (leave_msg.empty() ? "Leaving" : leave_msg) + "\r\n";

	for (; it != Server::_channels.end(); it++)
	{
		if (it->haveClient(client))
		{
			it->broadcast(msg, client);
			it->removeClient(client);
		}
		if (it->isClientInvite(client))
			it->removeInvite(client);
	}
}

void	Server::removeClient(Client &client)
{
	Server::_v.erase(std::remove(Server::_v.begin(), Server::_v.end(), client), Server::_v.end());
}

std::list<Channel>	Server::getAllChannels(void)
{
	return std::list<Channel>(Server::_channels);
}

void	Server::setPassword(const std::string &password)
{
	Server::_password = password;
}

const std::string	&Server::getPassword(void)
{
	const std::string &password = Server::_password;
	return password;
}

void	Server::setServerSocket(int socket)
{
	Server::_server_socket = socket;
}

int	Server::getSocket(void)
{
	return Server::_server_socket;
}

void	Server::setEpollFd(int epoll_fd)
{
	Server::_epoll_fd = epoll_fd;
}

int	Server::getEpollFd(void)
{
	return Server::_epoll_fd;
}

void	Server::clearFds(void)
{
	std::list<int>::iterator	it = Server::_fds.begin();

	for(; it != Server::_fds.end(); it++)
	{
		close(*it);
	}
	Server::_fds.clear();
}

void	Server::shutdown_server()
{
	std::vector<Client>::iterator	it = Server::_v.begin();
	std::string	msg = "ERROR: Server shutting down\r\n";


	for (; it != Server::_v.end(); it++)
	{
		sendServer(*it, RPL_CLOSE_CONNEXION(it->getNickname()));
		shutdown(it->getFd(), SHUT_RDWR);
		close(it->getFd());
	}

	close(Server::getEpollFd());
	shutdown(Server::getSocket(), SHUT_RDWR);
	close(Server::getSocket());

	std::cout << "---- SERVER SHUTDOWN -----" << std::endl;
}

const char	*Server::InvalidClientException::what(void) const throw()
{
	return ("Invalid Client");
}
