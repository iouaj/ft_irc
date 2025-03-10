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
		// Client	*_op;
		std::string	_name;
		bool	_invite_only;
		bool	_private;
		bool	_secret;
		bool	_topic_only_op;
		bool	_external_message;
		bool	_moderated;
		std::string	_password;
		std::list<Client>	_voice;
		std::list<Client>	_op;

	public:
		Channel(const Client &op, std::string name);
		~Channel(void);

		void	addOp(const Client &client);
		void	removeOp(const Client &client);
		bool	isOp(const Client &client) const;
		// Client	*getOp(void) const;

		const std::list<Client> &getClients(void) const;
		void	addClient(Client &Client);
		void	removeClient(Client &Client);

		void	setName(std::string name);
		const std::string &getName(void) const;

		bool	isInviteOnly(void) const;
		void	setInviteOnly(bool status);

		const std::string	&getPassword(void) const;
		void	setPassword(std::string password);

		void	setPrivate(bool status);
		bool	isPrivate(void) const;

		void	setSecret(bool status);
		bool	isSecret(void) const;

		void	setTopicOnlyOp(bool status);
		bool	isTopicOnlyOp(void) const;

		void	setExternalMessage(bool status);
		bool	isExternalMessage(void) const;

		void	addVoice(const Client &client);
		void	removeVoice(const Client &client);
		bool	hasClientVoice(const Client &client) const;

		void	setModerated(bool status);
		bool	isModerated(void) const;

		bool	haveClient(const Client &client) const;

		void	broadcast(std::string message) const;
		void	broadcast(std::string message, const Client &exclude) const;

		void kickMember(const Client &exec, const Client &target, std::string reason);


		bool operator==(const Channel &channel) const;

};

#endif
