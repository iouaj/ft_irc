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
		bool	_invite_only;
		bool	_private;
		bool	_secret;
		bool	_topic_only_op;
		bool	_external_message;
		bool	_moderated;

		std::string	_password;
		std::string	_name;
		std::string	_topic;

		std::list<Client>	_voice;
		std::list<Client>	_op;
		std::list<Client>	_banlist;
		std::list<Client> 	_clients;
		std::list<Client>	_invite;

		int	_limit;

	public:
		Channel(const Client &op, std::string name);
		~Channel(void);

		void	addOp(const Client &client);
		void	removeOp(const Client &client);
		bool	isOp(const Client &client) const;

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

		void	addBanList(const Client &client);
		void	removeBanList(const Client &client);
		bool	isBan(const Client &client) const;

		void		setLimit(int limit);
		const int	&getLimit(void) const;

		void	inviteClient(const Client &client);
		void	removeInvite(const Client &client);
		bool	isClientInvite(const Client &client) const;

		void	setTopic(const std::string &topic);
		const std::string	&getTopic(void) const;

		bool	haveClient(const Client &client) const;
		bool	haveClient(const std::string &name) const;

		const std::string	getSymbol(void) const;
		std::list<std::string>	getList(void) const;

		void	sendMode(const Client	&target) const;

		void	broadcast(std::string message) const;
		void	broadcast(std::string message, const Client &exclude) const;

		bool operator==(const Channel &channel) const;

};

#endif
