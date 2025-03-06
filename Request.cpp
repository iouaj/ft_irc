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
				this->param.push_back(token + rest);
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

		const std::string message = this->param[1].substr(1, this->param[1].size() - 2);
		const std::string channelName = this->param[0];

		if (Server::isChannelExist(channelName) == false) {
			send_error(client, ERR_CANNOTSENDTOCHAN, channelName, ":Can't send to this channel");
			return ;
		}

		const Channel *channel = Server::getChannel(channelName, &client);

		if (channel->haveClient(client) == false) {
			send_error(client, ERR_CANNOTSENDTOCHAN, channelName, ":Can't send to this channel");
			return ;
		}

		const std::list<Client> clients = channel->getClients();

		std::string msg = ":" + client.getNickname() + " PRIVMSG " + channel->getName() + " :" + message;
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

	Channel	*channel = Server::getChannel(this->param[0], &client);

	if (channel->isInviteOnly()) {
		send_error(client, ERR_INVITEONLYCHAN, channel->getName(), ": Channel is invite only");
		return ;
	}

	channel->addClient(client);

	send_priv(client, ":" + client.getNickname() + "!" + client.getUsername() + "@localhost" + " JOIN " + channel->getName());
}

void	Request::handleKick(int client_fd) const
{
	Client &client = Server::getClient(client_fd);
	Channel	*channel = Server::getChannel(this->param[0], &client);
	Client &target = Server::getClient(this->param[1]);

	std::string	reason = ":No reason";

	channel->kickMember(client, target, reason);
}

void	Request::handlePart(int client_fd) const
{
	Client &client = Server::getClient(client_fd);

	if (Server::isChannelExist(this->param[0])) {
		Channel *channel = Server::getChannel(this->param[0], &client);

		if (channel->haveClient(client) == false) {
			send_error(client, ERR_NOTONCHANNEL, channel->getName(), "You're not on that channel");
			return ;
		}

		channel->removeClient(client);
		std::string msg = ":" + client.getNickname() + "!" + client.getUsername() + "@localhost" + " PART " + channel->getName() + " :Leaving";
		channel->broadcast(msg);
		send_priv(client, msg);
	} else {
		send_error(client, ERR_NOSUCHCHANNEL, this->param[0], "No such channel");
	}

}

void	Request::exec(int client_fd) const
{
	if (!this->command.compare(0, 5, "NICK"))
	{
		this->nick(client_fd);
	} else if (!this->command.compare(0, 8, "PRIVMSG")) {
		this->privmsg(client_fd);
	} else if (!this->command.compare(0, 5, "USER")) {
		this->user(client_fd);	
	} else if (!this->command.compare(0, 4, "CAP")) {
		std::cout << "CAP ignored." << std::endl;
	} else if (!this->command.compare("MODE")) {
		this->mode(client_fd);
	} else if (!this->command.compare("PING")) {
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

