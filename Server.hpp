#ifndef SERVER_HPP
# define SERVER_HPP

#include <iostream>
#include <vector>
#include <list>
#include "Client.hpp"
#include "Channel.hpp"
#include "ft_irc.hpp"

class Channel;

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
		static std::list<Channel>	_channels;
		static std::string			_password;
		static	int					_server_socket;
		static	int					_epoll_fd;

		static std::list<int>	_fds;

	public:
		static	void	addClient(Client &client);
		static	void	removeClient(Client &client);
		static	Client	&getClient(int fd);
		static	Client	&getClient(std::string nickname);
		static Channel	*getChannel(std::string name, const Client &exec);
		static	void	addChannel(Channel &channel);
		static	void	deleteChannel(Channel &channel);
		static	bool	isChannelExist(std::string name);
		static	bool	isClientExist(std::string name);
		static	std::list<Channel>	getAllChannels(void);

		static	void	setServerSocket(int socket);
		static	int		getSocket(void);

		static void		setEpollFd(int epoll_fd);
		static	int		getEpollFd(void);

		static	void	clearFds(void);

		static	void	setPassword(const std::string &_password);
		static	const std::string &getPassword(void);

		static Channel	*createChannel(std::string &name, const Client &op);

		static void		clearClient(Client &client, const std::string &leave_msg);

		static void		shutdown_server(void);

};

#endif
