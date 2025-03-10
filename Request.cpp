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

void	Request::nick(int client_fd) const
{
	Client	&client = Server::getClient(client_fd);

	std::string	nickname;

	nickname = this->param[0];

	client.setNickname(nickname);

	std::string	message(":localhost 001 " + nickname + "\n");

	send(client_fd, message.c_str(), message.size(), 0);
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
			send_error(client, ERR_NOSUCHNICK, target_nickname, ":Can't find this nick");
			return ;
		}

		Client	&target = Server::getClient(target_nickname);

		const std::string message = this->param[1];

		send_priv(target, ":" + client.getNickname() + "!" + client.getUsername() + "@localhost" + " PRIVMSG " + target.getNickname() +  " " + message + "\n");
	}
}

void	Request::user(int client_fd) const
{
	Client &client = Server::getClient(client_fd);

	const std::string username = this->param[0];

	client.setUsername(username);
}

void	Request::mode(int client_fd) const
{
	Client &client = Server::getClient(client_fd);

	if (this->param[0][0] == '#' || this->param[0][0] == '@') //Channel
	{

		if (Server::isChannelExist(this->param[0]) == false) {
			send_error(client, ERR_NOSUCHCHANNEL, this->param[0], ":No such channel");
			return;
		}

		Channel	*channel = Server::getChannel(this->param[0], client);

		if (channel->isOp(client) == false) {
			send_error(client, ERR_CHANOPRIVSNEEDED, channel->getName(), ":You're not channel operator");
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
					send_error(client, ERR_NEEDMOREPARAMS, "MODE", ":Not enough parameters");
					return;
				}

				if (arg.size() > 20) {
					send_error(client, ERR_PASSWDMISMATCH, client.getNickname(), ":Password is too long");
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
					send_error(client, ERR_USERNOTINCHANNEL, arg, channel->getName() + " :They aren't on that channel");
					return;
				}

				const Client &target = Server::getClient(arg);

				if (channel->haveClient(target) == false) {
					send_error(client, ERR_USERNOTINCHANNEL, target.getNickname(), channel->getName() + " :They aren't on that channel");
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
					send_error(client, ERR_USERNOTINCHANNEL, arg, channel->getName() + " :They aren't on that channel");
					return;
				}

				const Client &target = Server::getClient(arg);

				if (channel->haveClient(target) == false) {
					send_error(client, ERR_USERNOTINCHANNEL, target.getNickname(), channel->getName() + " :They aren't on that channel");
					return;
				}

				mode[0] == '+' ? channel->addOp(target) : channel->removeOp(target);

				std::string msg = ":" + client.getNickname() + " MODE " + channel->getName() + " " + mode + " " + target.getNickname();
				channel->broadcast(msg);
				return;
			}
			default:
				break;
		}

		send_priv(client, ":" + client.getNickname() + " MODE " + channel->getName() + " " + mode);
	} else { //User

		const std::string username = this->param[0];

		if (username.compare(client.getNickname()))
		{
			send_priv(client, ":" + toStr(SERVER_NAME) + " " + toStr(ERR_USERSDONTMATCH) + ' ' + username + ' ' + ":Users don't match");
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
		send_error(client, ERR_NEEDMOREPARAMS, "JOIN", ":Not enough parameters");
		return;
	}

	Channel	*channel = Server::getChannel(this->param[0], client);

	if (channel->isInviteOnly()) {
		send_error(client, ERR_INVITEONLYCHAN, client.getNickname(), channel->getName() + " :Cannot join channel (+i)");
		return ;
	}

	std::string	password = channel->getPassword();

	if (password.empty() == false) {
		if (this->param.size() < 2 || this->param[1].compare(password)) {
			send_error(client, ERR_BADCHANNELKEY, client.getNickname(), channel->getName() + " :Cannot join channel (+k)");
			return;
		}
	}

	channel->addClient(client);

	send_priv(client, ":" + client.getNickname() + "!" + client.getUsername() + "@localhost" + " JOIN " + channel->getName());
}

void	Request::handleKick(int client_fd) const
{
	Client &client = Server::getClient(client_fd);
	Channel	*channel = Server::getChannel(this->param[0], client);
	Client &target = Server::getClient(this->param[1]);

	std::string	reason = ":No reason";

	channel->kickMember(client, target, reason);
}

void	Request::handlePart(int client_fd) const
{
	Client &client = Server::getClient(client_fd);

	if (Server::isChannelExist(this->param[0])) {
		Channel *channel = Server::getChannel(this->param[0], client);

		if (channel->haveClient(client) == false) {
			send_error(client, ERR_NOTONCHANNEL, channel->getName(), "You're not on that channel");
			return ;
		}

		channel->removeClient(client);
		std::string msg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost" + " PART " + channel->getName() + " :Leaving";
		channel->broadcast(msg);
		send_priv(client, msg);

		if (channel->getClients().size() == 0)
			Server::deleteChannel(*channel);

	} else {
		send_error(client, ERR_NOSUCHCHANNEL, this->param[0], "No such channel");
	}

}

void	Request::handleCap(int client_fd) const
{
	Client	client = Server::getClient(client_fd);

	if (!this->param[0].compare("LS")) {
		send_priv(client, ":localhost CAP * LS :\r\n");
	}
}

void	Request::exec(int client_fd) const
{
	if (!this->command.compare(0, 5, "NICK"))
		this->nick(client_fd);
	else if (!this->command.compare(0, 8, "PRIVMSG")) {
		this->privmsg(client_fd);
	} else if (!this->command.compare(0, 5, "USER")) {
		this->user(client_fd);
	} else if (!this->command.compare(0, 4, "CAP"))
		this->handleCap(client_fd);
	else if (!this->command.compare("MODE"))
		this->mode(client_fd);
	else if (!this->command.compare("PING")) {
		handlePing(client_fd);
	} else if (!this->command.compare("JOIN"))
		this->handleJoin(client_fd);
	else if (!this->command.compare("KICK"))
		this->handleKick(client_fd);
	else if (!this->command.compare("PART"))
		this->handlePart(client_fd);
	else {
		std::cout << "Command not found" << std::endl;
	}
}

