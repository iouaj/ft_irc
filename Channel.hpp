#ifndef CHANNEL_HPP
# define CHANNEL_HPP

#include <iostream>
#include <list>
#include <map>
#include "ft_irc.hpp"
#include "Client.hpp"

class Client;

class Channel
{
	private:
		std::list<Client> _clients;
		Client	*_op;
		std::string	_name;
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

		void	broadcast(std::string message) const;
		void	broadcast(std::string message, const Client &exclude) const;

		void kickMember(const Client &exec, const Client &target, std::string reason);


		bool operator==(const Channel &channel) const;

};

#endif
