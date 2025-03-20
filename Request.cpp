#include "ft_irc.hpp"
#include <vector>

const std::map<std::string, int> Request::validCommands = createMap();

std::map<std::string, int> Request::createMap(void) {
	std::map<std::string, int> m;
	m["NICK"] = 1;
	m["USER"] = 4;
	m["JOIN"] = 1;
	m["PART"] = 1;
	m["PRIVMSG"] = 2;
	m["QUIT"] = 0;
	m["CAP"] = 1;
	m["MODE"] = 1;
	m["PING"] = 1;
	m["NAMES"] = 0;
	m["KICK"] = 2;
	m["INVITE"] = 2;
	m["TOPIC"] = 1;
	m["PASS"] = 1;
	return m;
}

const char *Request::InvalidRequestException::what(void) const throw()
{
	return("Error : Invalid Request");
};

Request::Request(const char *buffer)
{
	this->str = std::string(buffer);

	std::istringstream iss(buffer);
	std::string	token;

	if (!(iss >> this->command)) {
		throw InvalidRequestException();
	}

	if (validCommands.find(this->command) == validCommands.end()) {
		throw InvalidRequestException();
	}

	while (iss >> token)
	{
		if (!token.empty() && token[0] == ':')
		{
			std::string rest;

			if (std::getline(iss, rest)) {
				this->param.push_back(clean_string(token.substr(1)) + clean_string(rest));
			}
			break;
		}
		this->param.push_back(token);
	}

	int requiredParams = validCommands.at(this->command);
	if ((int)this->param.size() < requiredParams) {
		throw InvalidRequestException();
	}

}

Request::~Request(void)
{}

void	Request::print(void) const
{
	std::cout << this->command;
	for (int i = 0; i < (int)this->param.size(); i++)
		std::cout << " " << this->param[i] ;
	std::cout << std::endl;
}

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

void	Request::privmsg(int client_fd) const
{
	Client &client = Server::getClient(client_fd);

	const std::string target_nickname = this->param[0];

	if (!target_nickname.compare(0, 1, "#")) { //Channel

		const std::string message = this->param[1];
		const std::string channelName = this->param[0];

		if (Server::isChannelExist(channelName) == false) {
			send_error(client, ERR_CANNOTSENDTOCHAN, channelName, ":Can't send to this channel");
			return ;
		}

		const Channel *channel = Server::getChannel(channelName, client);

		if (channel->isExternalMessage() == false && channel->haveClient(client) == false) {
			send_error(client, ERR_CANNOTSENDTOCHAN, channelName, ":Can't send to this channel (+n)");
			return ;
		}

		if (channel->isModerated() && channel->hasClientVoice(client) == false && channel->isOp(client) == false) {
			send_error(client, ERR_CANNOTSENDTOCHAN, channelName, ":Can't send to this channel (+m)");
			return ;
		}

		std::string msg = ":" + client.getNickname() + " PRIVMSG " + channel->getName() + " :" + message;
		if (msg.size() > 510) {
			std::cout << "Too long" << std::endl;
			send_error(client, ERR_INPUTTOOLONG, ":Message too long, max 510 bytes.", "");
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

		send_priv(target, ":" + client.getNickname() + "!" + client.getUsername() + "@localhost" + " PRIVMSG " + target.getNickname() +  " " + message + "\n");
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
					// send_error(client, ERR_NEEDMOREPARAMS, channel->getName(), ":Not enough parameters");
					// std::string msg = ":localhost " + toStr(ERR_NEEDMOREPARAMS) + " MODE " + channel->getName() + " +k" + " :Invalid Parameters";

					// sendTest(client, ERR_NEEDMOREPARAM(client.getNickname(), "MODE " + channel->getName() + " +k"));
					sendServer(client, ERR_NEEDMOREPARAM(client.getNickname(), "MODE"));
					// send_priv(client, msg);

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
					send_error(client, ERR_NEEDMOREPARAMS, "MODE", ":Not enough parameters");
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
					send_error(client, ERR_NEEDMOREPARAMS, "MODE", ":Not enough parameters");
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
					send_error(client, ERR_NEEDMOREPARAMS, "MODE", ":Not enough parameters");
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
					send_error(client, ERR_NEEDMOREPARAMS, "MODE", ":Not enough parameters");
					return;
				}

				if (isNumericString(arg) == false) {
					send_error(client, ERR_INVALIDMODEPARAM, arg, " :Invalid parameter");
					return;
				}

				int	limit = std::atoi(arg.c_str());

				if (limit > MAX_LIMIT_MEMBER_CHANNEL || limit <= 0) {
					send_error(client, ERR_INVALIDMODEPARAM, arg, ":Invalid parameter");
					return;
				}

				channel->setLimit(limit);

			}
			default:
				break;
		}

		send_priv(client, ":" + client.getNickname() + " MODE " + channel->getName() + " " + mode);
	} else { //User

		const std::string username = this->param[0];

		if (username.compare(client.getNickname()))
		{
			sendServer(client, ERR_USERSDONTMATCH(client.getNickname()));
			return ;
		}

		for (size_t i = 1; this->param.size() > i; i++)
		{
			if (this->param[i].size() != 2)
				continue;

			if (this->param[i][0] != '+' && this->param[i][0] != '-')
				continue;

			if (this->param[i][1] != 'i' && this->param[i][1] != 'w' && this->param[i][1] != 's' && this->param[i][1] != 'o')
				continue;

			if (this->param[i].compare("+i"))
			{
				client.setInvisible();
			}

			send_priv(client, ":" + toStr(SERVER_NAME) + " MODE " + username + " " + this->param[i]);
		}

	}
}

void	handlePing(int client_fd)
{
	Client &client = Server::getClient(client_fd);

	send_priv(client, ":" + toStr(SERVER_NAME) + " PONG " + toStr(SERVER_NAME));
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
	const std::string channel_name = this->param[0];
	const std::string target_name = this->param[1];

	if (Server::isChannelExist(channel_name) == false) {
		sendServer(client, ERR_NOSUCHCHANNEL(client.getNickname(), channel_name));
		return;
	}

	Channel	*channel = Server::getChannel(this->param[0], client);

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

	Client &target = Server::getClient(this->param[1]);

	std::string	reason = this->param.size() > 2 && this->param[2].empty() == false ? this->param[2] : "No reason";
	std::string msg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost" + " KICK " + channel->getName() + " " + target.getNickname() + " :" + reason;

	channel->broadcast(msg);
	channel->removeClient(target);
}

void	Request::handlePart(int client_fd) const
{
	Client &client = Server::getClient(client_fd);
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
		send_priv(client, msg);

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

	if (!this->param[0].compare("LS")) {
		send_priv(client, ":localhost CAP * LS :\r\n");
	}
}

void	Request::handleInvite(int client_fd) const
{
	Client	client = Server::getClient(client_fd);
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
	close(client.getFd());
	std::cout << client.getNickname() << " quit." << std::endl;
}

void	Request::handlePass(int client_fd) const
{
	Client &client = Server::getClient(client_fd);
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

	std::cout << client.getInvalidRequest() << std::endl;

	if (client.isSetup() == false) {
		if (this->command == "NICK")
			this->handleNick(client_fd);
		else if (this->command == "USER")
			this->handleUser(client_fd);
		else if (this->command == "CAP")
			this->handleCap(client_fd);
		else if (this->command == "PASS")
			this->handlePass(client_fd);
		else
			std::cout << "Command not found" << std::endl;

		if (client.getInvalidRequest() >= 10) {
			Server::clearClient(client, "");
			Server::removeClient(client);
			close(client.getFd());
			std::cout << "Client kick, not registered" << std::endl;
		}

		return;
	}


	if (!this->command.compare(0, 5, "NICK"))
		this->handleNick(client_fd);
	else if (!this->command.compare(0, 8, "PRIVMSG")) {
		this->privmsg(client_fd);
	} else if (!this->command.compare(0, 5, "USER")) {
		this->handleUser(client_fd);
	} else if (!this->command.compare(0, 4, "CAP"))
		this->handleCap(client_fd);
	else if (!this->command.compare("MODE"))
		this->handleMode(client_fd);
	else if (!this->command.compare("PING")) {
		handlePing(client_fd);
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
	else {
		std::cout << "Command not found" << std::endl;
	}
}

