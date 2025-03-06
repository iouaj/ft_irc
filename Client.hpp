#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <iostream>
#include "ft_irc.hpp"

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

        void    sendMessage(const Client &target, const std::string &message) const;
};

#endif