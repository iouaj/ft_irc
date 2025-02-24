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
	return m;
}

const char *Request::InvalidRequestException::what(void) const throw()
{
	return("Error : Invalid Request");
};

Request::Request(const char *buffer)
{
	// this->c_str = buffer;
	this->str = std::string(buffer);

	std::istringstream iss(buffer);
	std::string	token;

	if (!(iss >> this->command)) {
		std::cout << "1";
		throw InvalidRequestException();
	}

	if (validCommands.find(this->command) == validCommands.end()) {
		throw InvalidRequestException();
		std::cout << "2";
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
		std::cout << "SIZE PARAM " << this->param.size() << std::endl;
		std::cout << "3";
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

	std::cout << "TARGET NAME : " << target_nickname << std::endl;

	if (!target_nickname.compare(0, 1, "#")) { //Channel

		const std::string message = this->param[1].substr(1);
		const std::string channelName = this->param[0];

		std::cout << "CHANNEL NAME : " << channelName << std::endl;

		std::cout << "MESSAGE : " << message << std::endl;

		if (Server::isChannelExist(channelName) == false) {
			std::cout << "channel not exist" << std::endl;
			send_error(client, ERR_CANNOTSENDTOCHAN, channelName, ":Can't send to this channel");
			return ;
		}

		const Channel &channel = Server::getChannel(channelName);

		const std::list<Client> clients = channel.getClients();

		std::cout << "CLIENTS : ";
		for (std::list<Client>::const_iterator it = clients.begin(); it != clients.end(); it++ )
		{
			std::cout << it->getNickname() << " ";
		}
		std::cout << std::endl;

		send_group(channel.getClients(), ":" + client.getNickname() + " PRIVMSG " + channel.getName() + " :" + message + "\n", client);

	} else { //DM


		if (Server::isClientExist(target_nickname) == false) {
			send_error(client, ERR_NOSUCHNICK, target_nickname, ":Can't find this nick");
			return ;
		}

		Client	&target = Server::getClient(target_nickname);

		const std::string message = this->param[1];

		send_priv(target, ":" + client.getNickname() + " PRIVMSG " + target.getNickname() +  " :" + message + "\n");
		// send(target.getFd(), message.c_str(), message.size(), 0);
	}
}

void	Request::user(int client_fd) const
{

	// if (this->param.size() != 4)


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

	Channel	&channel = Server::getChannel(this->param[0]);

	std::cout << "check\n" << std::endl;

	if (channel.isInviteOnly()) {
		send_priv(client, ":" + toStr(SERVER_NAME) + " " + toStr(ERR_INVITEONLYCHAN) + " " + channel.getName() + " :Channel is invite-only");
		return ;
	}

	std::cout << "check2\n" << std::endl;

	channel.addClient(client);

	send_priv(client, ":" + toStr(SERVER_NAME) + " JOIN " + channel.getName());
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
	else {
		std::cout << "Command not found" << std::endl;
	}
}
