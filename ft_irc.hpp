#ifndef FT_IRC_HPP
# define FT_IRC_HPP

#include <cstring>
#include <string>
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
#include <list>
#include <algorithm>

#define SERVER_NAME	"localhost"

#define ERR_NOISSUE 001
#define ERR_NOSUCHNICK 401
#define ERR_NOSUCHCHANNEL 403
#define ERR_CANNOTSENDTOCHAN 404
#define ERR_NOTEXTTOSEND 412
#define ERR_NOTONCHANNEL 442
#define ERR_NEEDMOREPARAMS 461
#define ERR_INVITEONLYCHAN 473
#define ERR_CHANOPRIVSNEEDED 482
#define ERR_USERSDONTMATCH 502

template <typename T> std::string toStr(T tmp)
{
    std::ostringstream out;
    out << tmp;
    return out.str();
}

class Client;

class Channel
{
	private:
		std::list<Client> _clients;
		Client	*_op;
		std::string	_name;
		std::map<Client, std::string> _perms_client;
		std::string	_perms_channel;
		bool	_invite_only;
	
	public:
		Channel(Client *op, std::string name);
		~Channel(void);

		void	setOp(Client *admin);
		Client	*getOp(void) const;

		const std::list<Client> &getClients(void) const;
		void	addClient(Client &Client);
		void	removeClient(Client &Client);

		void	setName(std::string name);
		const std::string &getName(void) const;

		bool	isInviteOnly(void) const;
		bool	haveClient(const Client &client) const;

		const std::string	&getPermsChannel(void) const;
		void	setPermsChannel(std::string perms);

		void kickMember(const Client &exec, const Client &target, std::string reason);


		bool operator==(const Channel &channel) const;

};

class Client
{
	private:
		std::string _nickname;
		std::string	_username;
		int	_socket_fd;
		bool _visible;

	public:
		Client(const std::string &nickname, const int fd);
		~Client(void);

		void	setNickname(const std::string &nickname);
		void	setUsername(const std::string &username);
		const std::string	&getUsername(void) const;
		const std::string	&getNickname(void) const;
		void	setVisible(void);
		void	setInvisible(void);
		const bool	&getStatus(void);

		const	int	&getFd(void) const;

		bool operator==(const Client &client) const;
		bool operator!=(const Client &client) const;
};

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
		// static Channel	&default_channel;
		static std::list<Channel> _channels;

	public:
		static	void	addClient(Client &client);
		static	Client	&getClient(int fd);
		static	Client	&getClient(std::string nickname);
		// const static	Channel	&getDefaultChannel(void);
		// static void		setDefaultChannel(Channel &channel);
		static Channel	*getChannel(std::string name, Client *exec);
		// static Channel	&getChannel(std::string name, Client *exec);
		static	void	addChannel(Channel &channel);
		static	void	deleteChannel(Channel &channel);
		static	bool	isChannelExist(std::string name);
		static	bool	isClientExist(std::string name);

		static Channel	*createChannel(std::string name, Client *op);

};

class Request
{
	public:
		class InvalidRequestException : public std::exception
		{
			public:
				const char *what(void) const throw() ;
		};

	private:
		static std::map<std::string, int> createMap(void);

		static const std::map<std::string, int> validCommands;

		// char		*c_str;
		std::string	str;

		std::string	prefix;
		std::string	command;
		std::vector<std::string>	param;
		std::string	message;

		void	nick(int client_fd) const;
		void	privmsg(int client_fd) const;
		void	user(int client_fd) const;
		void	mode(int client_fd) const;
		void	handleJoin(int client_fd) const;
		void	handleKick(int client_fd) const;
		void	handlePart(int client_fd) const;

	public:
		Request(const char *buffer);
		~Request(void);

		void	exec(int client_fd) const;

		void print(void) const;

};

std::ostream &operator<<(std::ostream &os, const Request &req);


void    send_priv(const Client &client, std::string message);
void    send_group(const std::list<Client> &clients, std::string message, const Client &toSkip);
void    send_error(const Client &client, int error, std::string arg, std::string msg);
// void    send_error(const Client &client, int error, std::string arg, std::string msg);

#endif
