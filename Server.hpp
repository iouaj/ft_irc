#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>
#include <vector>
#include <list>
#include "Client.hpp"
#include "Channel.hpp"
#include "ft_irc.hpp"

class Server
{
	public:
		class InvalidClientException : std::exception
		{
			public:
				const char *what() const throw();
		};

	private:
		static std::vector<Client>	_v;
		static std::list<Channel> _channels;

	public:
		static	void	addClient(Client &client);
		static	Client	&getClient(int fd);
		static	Client	&getClient(std::string nickname);
		static Channel	*getChannel(std::string name, Client *exec);
		static	void	addChannel(Channel &channel);
		static	void	deleteChannel(Channel &channel);
		static	bool	isChannelExist(std::string name);
		static	bool	isClientExist(std::string name);

		static Channel	*createChannel(std::string name, Client *op);

};

#endif