#ifndef FT_IRC_HPP
# define FT_IRC_HPP

#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstddef>
#include <cstdlib>
#include <poll.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <map>
#include <sstream>
#include <vector>
#include <algorithm>

class Client
{
	private:
		std::string _nickname;
		const int	_socket_fd;

	public:
		Client(const std::string &nickname, const int fd);
		~Client(void);

		void	setNickname(const std::string &nickname);
		const std::string	&getNickname(void) const;

		const	int	&getFd(void) const;
};

class Server
{
	public:
		class InvalidClientException : std::exception
		{
			public:
				const char *what(void) throw();
		};

	private:
		static std::vector<Client>	_v;

	public:
		static	void	addClient(Client &client);
		static	Client	&getClient(int fd);
		static	Client	&getClient(std::string nickname);

};

class Request
{
	public:
		class InvalidRequestException : public std::exception
		{
			public:
				const char *what() const throw() ;
		};

	private:
		static std::map<std::string, int> createMap(void);

		static const std::map<std::string, int> validCommands;

		char		*c_str;
		std::string	str;

		std::string	prefix;
		std::string	command;
		std::vector<std::string>	param;
		std::string	message;

		void	nick(int client_fd) const;
		void	privmsg(int client_fd) const;

	public:
		Request(char *buffer);
		~Request(void);

		void	exec(int client_fd) const;

		void print(void) const;

};

std::ostream &operator<<(std::ostream &os, const Request &req);

#endif
