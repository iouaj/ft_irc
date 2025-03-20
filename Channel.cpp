#include "Channel.hpp"

Channel::Channel(const Client &op, std::string name) : _name(name)
{
	this->_op.push_back(op);
	this->_invite_only = false;
	this->_private = false;
	this->_secret = false;
	this->_topic_only_op = false;
	this->_external_message = false;
	this->_moderated = false;

	this->_limit = 0;
}

Channel::~Channel(void)
{

}

void	Channel::addOp(const Client &client)
{
	if (this->isOp(client) == false)
		this->_op.push_back(client);
}

void	Channel::removeOp(const Client &client)
{
	this->_op.remove(client);
}

bool	Channel::isOp(const Client &client) const
{
    return std::find(this->_op.begin(), this->_op.end(), client) != this->_op.end();
}

const std::list<Client>   &Channel::getClients(void) const
{
    const std::list<Client> &ref = this->_clients;
    return ref;
}

void	Channel::addClient(Client &client)
{
	if (this->haveClient(client) == false)
		this->_clients.push_back(client);
}

void	Channel::removeClient(Client &client)
{
	if (this->hasClientVoice(client))
		this->removeVoice(client);
	if (this->isOp(client))
		this->removeOp(client);
	this->_clients.remove(client);
}

void	Channel::setName(std::string name)
{
    this->_name = name;
}

const std::string	&Channel::getName(void) const
{
    const std::string &name = this->_name;
    return name;
}

bool    Channel::isInviteOnly(void) const
{
    return this->_invite_only == true;
}

void	Channel::setInviteOnly(bool status)
{
	this->_invite_only = status;
}

const std::string	&Channel::getPassword(void) const
{
	const std::string &password = this->_password;
	return password;
}

void	Channel::setPassword(std::string password)
{
	this->_password = password;
}

void	Channel::setPrivate(bool status)
{
	this->_private = status;
}

bool	Channel::isPrivate(void) const
{
	return this->_private == true;
}

void	Channel::setSecret(bool status)
{
	this->_secret = status;
}

bool	Channel::isSecret(void) const
{
	return this->_secret == true;
}

bool	Channel::isTopicOnlyOp(void) const
{
	return this->_topic_only_op == true;
}

void	Channel::setTopicOnlyOp(bool status)
{
	this->_topic_only_op = status;
}

void	Channel::setExternalMessage(bool status)
{
	this->_external_message = status;
}

bool	Channel::isExternalMessage(void) const
{
	return this->_external_message == true;
}

void	Channel::addVoice(const Client &client)
{
	if (this->hasClientVoice(client) == false)
		this->_voice.push_back(client);
}

void	Channel::removeVoice(const Client &client)
{
	this->_voice.remove(client);
}

bool	Channel::hasClientVoice(const Client &client) const
{
	return std::find(this->_voice.begin(), this->_voice.end(), client) != this->_voice.end();
}

void	Channel::setModerated(bool status)
{
	this->_moderated = status;
}

bool	Channel::isModerated(void) const
{
	return this->_moderated == true;
}

void	Channel::addBanList(const Client &client)
{
	if (this->isBan(client) == false)
		this->_banlist.push_back(client);
}

void	Channel::removeBanList(const Client &client)
{
	this->_banlist.remove(client);
}

bool	Channel::isBan(const Client &client) const
{
	return std::find(this->_banlist.begin(), this->_banlist.end(), client) != this->_banlist.end();
}

void	Channel::setLimit(int limit)
{
	this->_limit = limit;
}

const int	&Channel::getLimit(void) const
{
	const int	&limit = this->_limit;
	return limit;
}

void	Channel::inviteClient(const Client &client)
{
	if (this->isClientInvite(client) == false)
		this->_invite.push_back(client);
}

void	Channel::removeInvite(const Client &client)
{
	this->_invite.remove(client);
}

bool	Channel::isClientInvite(const Client &client) const
{
	return this->_invite.empty() == false && std::find(this->_invite.begin(), this->_invite.end(), client) != this->_invite.end();
}

void	Channel::setTopic(const std::string &topic)
{
	this->_topic = topic;
}

const std::string	&Channel::getTopic(void) const
{
	const std::string &topic = this->_topic;
	return topic;
}

bool	Channel::haveClient(const Client &client) const
{
	return std::find(this->_clients.begin(), this->_clients.end(), client) != this->_clients.end();
}

bool	Channel::haveClient(const std::string &name) const
{
	std::list<Client>::const_iterator	it = this->_clients.begin();

	for (; it != this->_clients.end(); it++)
	{
		if (it->getNickname() == name)
			return true;
	}
	return false;
}

bool	Channel::operator==(const Channel &channel) const
{
	return this->_name == channel.getName();
}

const std::string	Channel::getSymbol(void) const
{
	return (isPrivate() || isSecret()) ? "@" : isModerated() ? "m" : "=";
}

std::list<std::string>	Channel::getList(void) const
{
	std::list<std::string>	names;
	std::list<Client>::const_iterator it = this->_clients.begin();

	for (; it != this->_clients.end(); it++)
	{
		names.push_back((this->isOp(*it) ? "@" : this->hasClientVoice(*it) ? "+" : "") + it->getNickname());
	}

	return names;
}

void	Channel::sendMode(const Client &target) const
{
	std::string	mode;

	this->isInviteOnly() ? mode.append("+i ") : mode.append("-i ");
	this->isExternalMessage() ? mode.append("+n ") : mode.append("-n ");
	this->isPrivate() ? mode.append("+p ") : mode.append("-p ");
	this->isModerated() ? mode.append("+m ") : mode.append("-m ");
	this->isSecret() ? mode.append("+s ") : mode.append("-s ");
	this->isTopicOnlyOp() ? mode.append("+t ") : mode.append("-t ");
	this->getLimit() != 0 ? mode.append("+l " + toStr(this->getLimit()) + " ") : mode.append("-l ");
	this->getPassword().empty() == false ? mode.append("+k " + this->getPassword() + " ") : mode.append("-k ");

	mode += "\r\n";

	sendServer(target, RPL_CHANNELMODEIS(target.getNickname(), this->getName(), mode));
}

void	Channel::broadcast(std::string message) const
{
	std::list<Client>::const_iterator it = this->_clients.begin();

	message = message + "\r\n";
	for (; it != this->_clients.end(); it++)
	{
		send(it->getFd(), message.c_str(), message.size(), 0);
	}
}

void	Channel::broadcast(std::string message, const Client &exclude) const
{
	std::list<Client>::const_iterator it = this->_clients.begin();

	message = message + "\r\n";
	for (; it != this->_clients.end(); it++)
	{
		if (*it == exclude)
			continue;
		send(it->getFd(), message.c_str(), message.size(), 0);
	}
}
