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
				this->param.push_back(token.substr(1) + rest);
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

	std::string	message(":localhost 001 " + nickname + ":Welcome\n");

	send(client_fd, message.c_str(), message.size(), 0);
}

void	Request::privmsg(int client_fd) const
{
	Client &client = Server::getClient(client_fd);
	// (void) client;

	const std::string target_nickname = this->param[0];

	if (!target_nickname.compare(0, 1, "#")) { //Channel

		// if (this->param[1].c_str()[0] != ':') {
		// 	std::cerr << "Syntax Invalide" << std::endl;
		// 	return ;
		// }

		const std::string message = this->param[1];


	} else { //DM

		// if (this->param[1].c_str()[0] != ':') {
		// 	std::cerr << "Syntax Invalide" << std::endl;
		// 	return ;
		// }

		Client	&target = Server::getClient(target_nickname);

		const std::string message = this->param[1];

		send_priv(target, client.getNickname() + " : " + message + "\n");
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

// void	Request::mode

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
	} else {
		std::cout << "Command not found" << std::endl;
	}
}
