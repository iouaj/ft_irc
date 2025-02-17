#include "ft_irc.hpp"

void    send_priv(const Client &client, std::string message)
{

    message = "\033[1;36m" + message + "\033[0;0m";
    if (send(client.getFd(), message.c_str(), message.size(), 0) == -1)
    {
        std::cerr << "Error: Message can't be send" << std::endl;
    }
}

void    send_group(const std::vector<Client> &clients, std::string message)
{
    for (std::vector<Client>::const_iterator it = clients.begin(); it != clients.end(); it++)
    {
        if (send(it->getFd(), message.c_str(), message.size(), 0) == -1)
        {
            std::cerr << "Error: Message can't be send to " << it->getNickname() << std::endl;
        }
    }
}
