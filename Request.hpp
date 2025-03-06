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
		static std::map<std::string, int> createMap(void);

		static const std::map<std::string, int> validCommands;

		std::string	str;

		std::string	command;
		std::vector<std::string>	param;

		void	nick(int client_fd) const;
		void	privmsg(int client_fd) const;
		void	user(int client_fd) const;
		void	mode(int client_fd) const;
		void	handleJoin(int client_fd) const;
		void	handleKick(int client_fd) const;
		void	handlePart(int client_fd) const;
		void	handleCap(int client_fd) const;

	public:
		Request(const char *buffer);
		~Request(void);

		void	exec(int client_fd) const;

		void print(void) const;

};

#endif
