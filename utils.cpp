#include "ft_irc.hpp"

void    send_priv(const Client &client, std::string message)
{
    message = message + "\r\n";
    if (send(client.getFd(), message.c_str(), message.size(), 0) == -1)
    {
        std::cerr << "Error: Message can't be send" << std::endl;
    }
}

void    send_error(const Client &client, int error, std::string arg, std::string msg)
{
    std::string message = ":" + toStr(SERVER_NAME) + " " + toStr(error) + " " + arg + " " + msg + "\n";
    if (send(client.getFd(), message.c_str(), message.size(), 0) == -1)
    {
        std::cerr << "Error: Message can't be send" << std::endl;
    }
}

void    send_group(const std::list<Client> &clients, std::string message, const Client &toSkip)
{
    std::cout << "MESSAGE : " << message << std::endl;
    for (std::list<Client>::const_iterator it = clients.begin(); it != clients.end(); it++)
    {
        if (*it == toSkip)
            continue;
        if (send(it->getFd(), message.c_str(), message.size(), 0) == -1)
        {
            std::cerr << "Error: Message can't be send to " << it->getNickname() << std::endl;
        }
    }
}

std::string	clean_string(std::string str)
{
	std::string clean("");
	for (std::size_t i = 0; i <= str.size(); i++)
	{
		if (std::isprint(str[i]) && str[i] != '\n' && str[i] != '\r' && str[i] != 0) {
			clean += str[i];
		}
	}
	return clean;
}
