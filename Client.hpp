#ifndef CLIENT_HPP
# define CLIENT_HPP

#include <iostream>
#include "ft_irc.hpp"

class Client
{
	private:
		std::string _nickname;
		std::string	_username;
		int			_socket_fd;
		bool		_visible;
		bool		_setup;
		bool		_pass;

		int		_invalid_request;

	public:
		Client(const std::string &nickname, const int fd);
		~Client(void);

		void	setNickname(const std::string &nickname);
		void	setUsername(const std::string &username);
		const std::string	&getUsername(void) const;
		const std::string	&getNickname(void) const;

		void	setVisible(void);
		void	setInvisible(void);

		bool	isVisible(void) const;

		void	setup(void);
		bool	isSetup(void) const;

		void	pass(void);
		bool	isPass(void) const;

		const	int	&getFd(void) const;

		void	incrInvalidRequest(void);
		const	int	&getInvalidRequest(void) const;

		bool operator==(const Client &client) const;
		bool operator!=(const Client &client) const;

		void	sendEverywhere(const std::string &message) const;
		void	sendMessage(const Client &target, const std::string &message) const;
};

#endif
