#include "ft_irc.hpp"
#include <vector>

const char *Request::InvalidRequestException::what(void) const throw()
{
	return("Error : Invalid Request");
};

Request::Request(const char *buffer)
{
	std::istringstream iss(buffer);
	std::string	token;

	if (!(iss >> this->command)) {
		throw InvalidRequestException();
	}

	while (iss >> token)
	{
		std::cout << token << std::endl;
		if (!token.empty() && token[0] == ':')
		{
			std::string rest;

			if (std::getline(iss, rest)) {
				this->param.push_back((token.substr(1)) + rest.substr(0, rest.size() - 2));
			}
			break;
		}
		this->param.push_back(token);
	}
}

Request::~Request(void)
{}

bool	isNickValid(const std::string &nickname) {

	if (nickname.empty())
		return false;

	for (std::size_t i = 0; i < nickname.size(); i++)
	{
		if (std::isalpha(nickname[i]) == false)
			return false;
	}

	return nickname.size() <= 20;
}

void	Request::handleNick(int client_fd) const
{
	Client	&client = Server::getClient(client_fd);

	if (this->param.size() < 1) {
		sendServer(client, ERR_NEEDMOREPARAM(client.getNickname(), "NICK"));
		return;
	}

	if (client.isPass() == false) {
		sendServer(client, ERR_NOTREGISTERED);
		client.incrInvalidRequest();
		return;
	}

	std::string	nickname;

	nickname = this->param[0];

	if (isNickValid(nickname) == false) {
		sendServer(client, ERR_ERRONEUSNICKNAME(client.getNickname().empty() ? "*" : client.getNickname(), nickname));
		client.incrInvalidRequest();
		return;
	}

	if (Server::isClientExist(nickname)) {
		sendServer(client, ERR_NICKNAMEINUSE(client.getNickname().empty() ? "*" : client.getNickname(), nickname));
		client.incrInvalidRequest();
		return;
	}

	if (client.isSetup() == false) {
		client.setNickname(nickname);
		return;
	}

	std::string msg = ":" + client.getNickname() + " NICK " + nickname;

	client.setNickname(nickname);

	client.sendEverywhere(msg);
}

bool	isTextValid(const std::string &text)
{
	for (std::size_t i = 0; i < text.size(); i++)
	{
		if (std::isprint(text[i]) == false)
			return false;
	}
	return true;
}

void	Request::handlePrivmsg(int client_fd) const
{
	Client &client = Server::getClient(client_fd);

	if (this->param.size() < 2) {
		sendServer(client, ERR_NEEDMOREPARAM(client.getNickname(), "PRIVMSG"));
		return;
	}

	const std::string target_nickname = this->param[0];

	if (!target_nickname.compare(0, 1, "#")) { //Channel

		const std::string message = this->param[1];
		const std::string channelName = this->param[0];

		std::cout << "message " << message;
		if (message.empty() || isTextValid(message) == false) {
			sendServer(client, ERR_NOTEXTTOSEND(client.getNickname()));
			return;
		}

		if (Server::isChannelExist(channelName) == false) {
			sendServer(client, ERR_NOSUCHNICK(client.getNickname(), channelName));
			return;
		}

		const Channel *channel = Server::getChannel(channelName, client);

		if (channel->isExternalMessage() == false && channel->haveClient(client) == false) {
			sendServer(client, ERR_CANNOTSENDTOCHAN(client.getNickname(), channelName));
			return;
		}

		if (channel->isModerated() && channel->hasClientVoice(client) == false && channel->isOp(client) == false) {
			sendServer(client, ERR_CANNOTSENDTOCHAN(client.getNickname(), channelName));
			return;
		}

		std::string msg = ":" + client.getNickname() + " PRIVMSG " + channel->getName() + " :" + message;
		if (msg.size() > 510) {
			sendServer(client, ERR_INPUTTOOLONG(client.getNickname()));
			return;
		}
		channel->broadcast(msg, client);

	} else { //DM


		if (Server::isClientExist(target_nickname) == false) {
			sendServer(client, ERR_NOSUCHNICK(client.getNickname(), target_nickname));
			return ;
		}

		Client	&target = Server::getClient(target_nickname);

		const std::string message = this->param[1];

		if (message.empty()) {
			sendServer(client, ERR_NOTEXTTOSEND(client.getNickname()));
			return;
		}

		client.sendMessage(target, message);
	}
}

bool	isUsernameValid(const std::string username)
{
	if (username.empty())
		return false;
	for (std::size_t i = 0; i < username.size(); i++)
	{
		char c = username[i];
		if ((std::isalnum(c) || c == '-' || c == '_') == false)
			return false;
	}
	return username.size();
}

void	Request::handleUser(int client_fd) const
{
	Client &client = Server::getClient(client_fd);

	if (this->param.size() < 1) {
		sendServer(client, ERR_NEEDMOREPARAM(client.getNickname(), "USER"));
		return;
	}

	if (client.getNickname().empty() || client.isPass() == false) {
		sendServer(client, ERR_NOTREGISTERED);
		client.incrInvalidRequest();
		return;
	}

	if (client.getUsername().empty() == false) {
		sendServer(client, ERR_ALREADYREGISTRED(client.getNickname()));
		return;
	}

	const std::string username = this->param[0];

	if (isUsernameValid(username) == false) {
		sendServer(client, ERR_NEEDMOREPARAM(client.getNickname(), "USER"));
		client.incrInvalidRequest();
		return;
	}

	client.setUsername(username);

	client.setup();
}

void	Request::handleMode(int client_fd) const
{
	Client &client = Server::getClient(client_fd);

	if (this->param.size() < 1) {
		sendServer(client, ERR_NEEDMOREPARAM(client.getNickname(), "MODE"));
		return;
	}

	if (this->param[0][0] == '#' || this->param[0][0] == '@') //Channel
	{

		const std::string	channel_name = this->param[0];

		if (Server::isChannelExist(channel_name) == false) {
			sendServer(client, ERR_NOSUCHCHANNEL(client.getNickname(), channel_name));
			return;
		}

		Channel	*channel = Server::getChannel(channel_name, client);

		if (this->param.size() < 2) { //Ask Channel's mode
			channel->sendMode(client);
			return;
		}

		if (channel->isOp(client) == false) {
			sendServer(client, ERR_CHANOPRIVSNEEDED(client.getNickname(), channel_name));
			return;
		}

		const std::string mode = this->param[1];
		const std::string arg = this->param.size() > 2 ? this->param[2] : "";

		switch (mode[1])
		{
			case 'i':
				channel->setInviteOnly(mode[0] == '+');
				break;

			case 'k': //A revoir

				if (arg.empty()) {
					sendServer(client, ERR_NEEDMOREPARAM(client.getNickname(), "MODE"));
					return;
				}

				if (arg.size() > 20) {
					sendServer(client, ERR_PASSWDMISMATCH(client.getNickname()));
					return;
				}

				channel->setPassword(mode[0] == '-' ? "" : arg);
				break;

			case 'p':
				channel->setPrivate(mode[0] == '+');
				break;

			case 's':
				channel->setSecret(mode[0] == '+');
				break;

			case 't':
				channel->setTopicOnlyOp(mode[0] == '+');
				break;

			case 'n':
				channel->setExternalMessage(mode[0] == '+');
				break;

			case 'm':
				channel->setModerated(mode[0] == '+');
				break;

			case 'v': {

				if (arg.empty()) {
					sendServer(client, ERR_NEEDMOREPARAM(client.getNickname(), "MODE"));
					return;
				}

				if (Server::isClientExist(arg) == false) {
					sendServer(client, ERR_USERNOTINCHANNEL(client.getNickname(), arg, channel->getName()));
					return;
				}

				Client &target = Server::getClient(arg);

				if (channel->haveClient(target) == false) {
					sendServer(client, ERR_USERNOTINCHANNEL(client.getNickname(), target.getNickname(), channel->getName()));
					return;
				}

				mode[0] == '+' ? channel->addVoice(target) : channel->removeVoice(target);

				std::string msg = ":" + client.getNickname() + " MODE " + channel->getName() + " " + mode + " " + target.getNickname();
				channel->broadcast(msg);
				return;
			}

			case 'o': {
				if (arg.empty()) {
					sendServer(client, ERR_NEEDMOREPARAM(client.getNickname(), "MODE"));
					return;
				}

				if (Server::isClientExist(arg) == false) {
					sendServer(client, ERR_USERNOTINCHANNEL(client.getNickname(), arg, channel->getName()));
					return;
				}

				Client &target = Server::getClient(arg);

				if (channel->haveClient(target) == false) {
					sendServer(client, ERR_USERNOTINCHANNEL(client.getNickname(), target.getNickname(), channel->getName()));
					return;
				}

				mode[0] == '+' ? channel->addOp(target) : channel->removeOp(target);

				std::string msg = ":" + client.getNickname() + " MODE " + channel->getName() + " " + mode + " " + target.getNickname();
				channel->broadcast(msg);
				return;
			}

			case 'b': {
				if (arg.empty()) {
					sendServer(client, ERR_NEEDMOREPARAM(client.getNickname(), "MODE"));
					return;
				}

				if (Server::isClientExist(arg) == false) {
					sendServer(client, ERR_USERNOTINCHANNEL(client.getNickname(), arg, channel->getName()));
					return;
				}

				Client &target = Server::getClient(arg);

				if (channel->haveClient(target) == false) {
					sendServer(client, ERR_USERNOTINCHANNEL(client.getNickname(), arg, channel->getName()));
					return;
				}

				mode[0] == '+' ? channel->addBanList(client) : channel->removeBanList(client);
				std::string msg = ":" + client.getNickname() + " MODE " + channel->getName() + " " + mode + " " + target.getNickname();
				channel->broadcast(msg);
				return;
			}

			case 'l': {

				if (mode[0] == '-') {
					channel->setLimit(0);
					break;
				}

				if (arg.empty()) {
					sendServer(client, ERR_NEEDMOREPARAM(client.getNickname(), "MODE"));
					return;
				}

				if (isNumericString(arg) == false) {
					sendServer(client, ERR_INVALIDMODEPARAM(client.getNickname(), channel->getName(), arg));
					return;
				}

				int	limit = std::atoi(arg.c_str());

				if (limit > MAX_LIMIT_MEMBER_CHANNEL || limit <= 0) {
					sendServer(client, ERR_INVALIDMODEPARAM(client.getNickname(), channel->getName(), arg));
					return;
				}

				channel->setLimit(limit);
				break;
			}
			default:
				sendServer(client, ERR_UNKNOWNMODE(client.getNickname(), mode));
				return;
		}

		sendServer(client, ":" + client.getNickname() + " MODE " + channel->getName() + " " + mode);
	}
}

void	Request::handlePing(int client_fd) const
{
	Client &client = Server::getClient(client_fd);

	if (this->param.size() < 1) {
		sendServer(client, ERR_NEEDMOREPARAM(client.getNickname(), "PING"));
		return;
	}

	sendServer(client, RPL_PONG(this->param[0]));
}

void  Request::handleJoin(int client_fd) const
{
	Client	&client = Server::getClient(client_fd);

	if (this->param.empty() || this->param[0].empty()) {
		sendServer(client, ERR_NEEDMOREPARAM(client.getNickname(), "JOIN"));
		return;
	}

	Channel	*channel = Server::getChannel(this->param[0], client);

	if (channel->isInviteOnly() && channel->isClientInvite(client) == false) {
		sendServer(client, ERR_INVITEONLYCHAN(client.getNickname(), channel->getName()));
		return ;
	}

	const int	&limit = channel->getLimit();

	if (limit != 0 && limit <= (int)channel->getClients().size()) {
		sendServer(client, ERR_CHANNELISFULL(client.getNickname(), channel->getName()));
		return;
	}

	std::string	password = channel->getPassword();

	if (password.empty() == false) {
		if (this->param.size() < 2 || this->param[1].compare(password)) {
			sendServer(client, ERR_BADCHANNELKEY(client.getNickname(), channel->getName()));
			return;
		}
	}

	if (channel->isBan(client)) {
		sendServer(client, ERR_BANNEDFROMCHAN(client.getNickname(), channel->getName()));
		return;
	}

	channel->addClient(client);

	if (channel->isClientInvite(client)) {
		channel->removeInvite(client);
	}

	const std::string msg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost" + " JOIN " + channel->getName();
	channel->broadcast(msg);

	sendServer(client, RPL_TOPIC(client.getNickname(), channel->getName(), channel->getTopic()));
	sendNameReply(client, *channel);
}

void	Request::handleKick(int client_fd) const
{
	Client &client = Server::getClient(client_fd);

	if (this->param.size() < 2) {
		sendServer(client, ERR_NEEDMOREPARAM(client.getNickname(), "KICK"));
		return;
	}

	const std::string channel_name = this->param[0];
	const std::string target_name = this->param[1];

	if (Server::isChannelExist(channel_name) == false) {
		sendServer(client, ERR_NOSUCHCHANNEL(client.getNickname(), channel_name));
		return;
	}

	Channel	*channel = Server::getChannel(channel_name, client);

	if (channel->haveClient(client) == false) {
		sendServer(client, ERR_NOTONCHANNEL(client.getNickname(), channel_name));
		return;
	}

	if (channel->isOp(client) == false) {
		sendServer(client, ERR_CHANOPRIVSNEEDED(client.getNickname(), channel_name));
		return;
	}

	if (channel->haveClient(target_name) == false) {
		sendServer(client, ERR_USERNOTINCHANNEL(client.getNickname(), target_name, channel_name));
		return;
	}

	Client &target = Server::getClient(target_name);

	std::string	reason = this->param.size() > 2 && this->param[2].empty() == false ? this->param[2] : "No reason";
	std::string msg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost" + " KICK " + channel->getName() + " " + target.getNickname() + " :" + reason;

	channel->broadcast(msg);
	channel->removeClient(target);
}

void	Request::handlePart(int client_fd) const
{
	Client &client = Server::getClient(client_fd);

	if (this->param.size() < 1) {
		sendServer(client, ERR_NEEDMOREPARAM(client.getNickname(), "PART"));
		return;
	}

	const std::string	channel_name = this->param[0];

	if (Server::isChannelExist(channel_name)) {
		Channel *channel = Server::getChannel(channel_name, client);

		if (channel->haveClient(client) == false) {
			sendServer(client, ERR_NOTONCHANNEL(client.getNickname(), channel->getName()));
			return ;
		}

		channel->removeClient(client);
		std::string msg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost" + " PART " + channel->getName() + " :Leaving";
		channel->broadcast(msg);
		sendServer(client, msg);

		if (channel->getClients().size() == 0)
			Server::deleteChannel(*channel);

	} else {
		sendServer(client, ERR_NOSUCHCHANNEL(client.getNickname(), channel_name));
	}

}

void	Request::handleCap(int client_fd) const
{
	Client	client = Server::getClient(client_fd);

	if (client.isPass() == false) {
		sendServer(client, ERR_NOTREGISTERED);
		client.incrInvalidRequest();
		return;
	}

	if (this->param.size() < 1) {
		sendServer(client, ERR_NEEDMOREPARAM(client.getNickname(), "CAP"));
		return;
	}

	if (!this->param[0].compare("LS")) {
		sendServer(client, ":localhost CAP * LS :\r\n");
	}
}

void	Request::handleInvite(int client_fd) const
{
	Client	client = Server::getClient(client_fd);

	if (this->param.size() < 2) {
		sendServer(client, ERR_NEEDMOREPARAM(client.getNickname(), "INVITE"));
		return;
	}

	const std::string	target_nick = this->param[0];

	if (Server::isClientExist(target_nick) == false) {
		sendServer(client, ERR_NOSUCHNICK(client.getNickname(), target_nick));
		return;
	}

	Client	target = Server::getClient(target_nick);

	const std::string	channel_name = this->param[1];

	if (Server::isChannelExist(channel_name) == false) {
		sendServer(client, ERR_NOSUCHCHANNEL(client.getNickname(), channel_name));
		return;
	}

	Channel	*channel = Server::getChannel(channel_name, client);

	if (channel->haveClient(client) == false) {
		sendServer(client, ERR_NOTONCHANNEL(client.getNickname(), channel_name));
		return;
	}

	if (channel->haveClient(target) == true) {
		sendServer(client, ERR_USERONCHANNEL(client.getNickname(), channel_name));
		return;
	}

	if (channel->isInviteOnly() == true && channel->isOp(client) == false) {
		sendServer(client, ERR_CHANOPRIVSNEEDED(client.getNickname(), channel_name));
		return;
	}

	if (channel->isClientInvite(target) == false) {
		channel->inviteClient(target);
	}

	sendServer(client, RPL_INVITING(client.getNickname(), channel_name, target_nick));
	sendServer(target, ":" + client.getNickname() + "!" + client.getUsername() + "@localhost INVITE " + target.getNickname() + " :" + channel->getName() + "\r\n");
}

void	Request::handleTopic(int client_fd) const
{
	Client	&client = Server::getClient(client_fd);

	if (this->param.size() < 1) {
		sendServer(client, ERR_NEEDMOREPARAM(client.getNickname(), "TOPIC"));
		return;
	}

	const	std::string	channel_name = this->param[0];

	if (Server::isChannelExist(channel_name) == false) {
		sendServer(client, ERR_NOSUCHCHANNEL(client.getNickname(), channel_name));
		return;
	}

	Channel	*channel = Server::getChannel(channel_name, client);

	if (channel->haveClient(client) == false) {
		sendServer(client, ERR_NOTONCHANNEL(client.getNickname(), channel_name));
		return;
	}

	if (this->param.size() < 2) { //Give Topic

		const std::string &topic = channel->getTopic();

		if (topic.empty() == true) {
			sendServer(client, RPL_NOTOPIC(client.getNickname(), channel_name));
			return;
		}

		sendServer(client, RPL_TOPIC(client.getNickname(), channel_name, topic));
		return;

	} else { //Set Topic

		if (channel->isTopicOnlyOp() == true && channel->isOp(client) == false) {
			sendServer(client, ERR_CHANOPRIVSNEEDED(client.getNickname(), channel_name));
			return;
		}

		const std::string	&new_topic = this->param[1];

		if (new_topic.empty() == true) {
			sendServer(client, ERR_NEEDMOREPARAM(client.getNickname(), "TOPIC"));
			return;
		}

		channel->setTopic(new_topic);

		const std::string msg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost TOPIC " + channel_name + " :" + new_topic;
		channel->broadcast(msg);
		return;
	}
}

void	Request::handleQuit(int client_fd) const
{
	Client	&client = Server::getClient(client_fd);
	std::string msg;

	if (this->param.size() > 0)
		msg = this->param[0];

	Server::clearClient(client, msg);
	Server::removeClient(client);
	close(client.getFd());
	std::cout << client.getNickname() << " quit." << std::endl;
}

void	Request::handlePass(int client_fd) const
{
	Client &client = Server::getClient(client_fd);

	if (this->param.size() < 1) {
		sendServer(client, ERR_NEEDMOREPARAM(client.getNickname(), "PASS"));
		return;
	}

	const std::string	passwd = this->param[0];

	if (client.isSetup() || client.isPass()) {
		sendServer(client, ERR_ALREADYREGISTRED(client.getNickname().empty() ? "*" : client.getNickname()));
		return;
	}

	if (Server::getPassword() != passwd) {
		sendServer(client, ERR_PASSWDMISMATCH(client.getNickname().empty() ? "*" : client.getNickname()));
		return;
	}

	client.pass();
}

void	Request::exec(int client_fd) const
{

	Client	&client = Server::getClient(client_fd);

	if (client.isSetup() == false) {
		if (this->command == "NICK")
			this->handleNick(client_fd);
		else if (this->command == "USER")
			this->handleUser(client_fd);
		else if (this->command == "CAP")
			this->handleCap(client_fd);
		else if (this->command == "PASS")
			this->handlePass(client_fd);
		else {
			sendServer(client, ERR_UNKNOWNCOMMAND(client.getNickname(), this->command));
			client.incrInvalidRequest();
		}

		if (client.getInvalidRequest() >= 10) {
			Server::clearClient(client, "");
			Server::removeClient(client);
			close(client.getFd());
			std::cout << "Client kick, not registered" << std::endl;
		}

		return;
	}

	if (!this->command.compare("NICK"))
		this->handleNick(client_fd);
	else if (!this->command.compare("PRIVMSG")) {
		this->handlePrivmsg(client_fd);
	} else if (!this->command.compare("USER")) {
		this->handleUser(client_fd);
	} else if (!this->command.compare("CAP"))
		this->handleCap(client_fd);
	else if (!this->command.compare("MODE"))
		this->handleMode(client_fd);
	else if (!this->command.compare("PING")) {
		this->handlePing(client_fd);
	} else if (!this->command.compare("JOIN"))
		this->handleJoin(client_fd);
	else if (!this->command.compare("KICK"))
		this->handleKick(client_fd);
	else if (!this->command.compare("PART"))
		this->handlePart(client_fd);
	else if (!this->command.compare("INVITE"))
		this->handleInvite(client_fd);
	else if (!this->command.compare("TOPIC"))
		this->handleTopic(client_fd);
	else if (!this->command.compare("QUIT"))
		this->handleQuit(client_fd);
	else
		sendServer(client, ERR_UNKNOWNCOMMAND(client.getNickname(), this->command));
}

