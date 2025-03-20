#ifndef REQUEST_HPP
# define REQUEST_HPP

#include <iostream>
#include <map>
#include <vector>
#include "ft_irc.hpp"

class Request
{
	public:
		class InvalidRequestException : public std::exception
		{
			public:
				const char *what(void) const throw() ;
		};

	private:
		std::string					command;
		std::vector<std::string>	param;

		void	handleNick(int client_fd) const;
		void	handlePrivmsg(int client_fd) const;
		void	handleUser(int client_fd) const;
		void	handleMode(int client_fd) const;
		void	handleJoin(int client_fd) const;
		void	handleKick(int client_fd) const;
		void	handlePart(int client_fd) const;
		void	handleCap(int client_fd) const;
		void	handleInvite(int client_fd) const;
		void	handleTopic(int client_fd) const;
		void	handleQuit(int client_fd) const;
		void	handlePass(int client_fd) const;
		void	handlePing(int client_fd) const;

	public:
		Request(const char *buffer);
		~Request(void);

		void	exec(int client_fd) const;
};

#endif
