#include "ft_irc.hpp"

void    send_priv(const Client &client, std::string message)
{
    message = "\033[1;36m" + message + "\033[0;0m\n";
    if (send(client.getFd(), message.c_str(), message.size(), 0) == -1)
    {
        std::cerr << "Error: Message can't be send" << std::endl;
    }
}

// void    send_error(const Client &client, int error, std::string arg, std::string msg)
// {
//     std::string message = ":ircserv " + error + " " + arg + " :" + msg;
//     if (send(client.getFd(), message.c_str(), message.size(), 0) == -1)
//     {
//         std::cerr << "Error: Message can't be send" << std::endl;
//     }
// }

void    send_error(const Client &client, int error, std::string arg, std::string msg)
{
    std::string message = "\033[1;31m:" + toStr(SERVER_NAME) + " " + toStr(error) + " " + arg + " " + msg + "\033[0;0m\n";
    if (send(client.getFd(), message.c_str(), message.size(), 0) == -1)
    {
        std::cerr << "Error: Message can't be send" << std::endl;
    }
}

void    send_group(const std::list<Client> &clients, std::string message, const Client &toSkip)
{
    for (std::list<Client>::const_iterator it = clients.begin(); it != clients.end(); it++)
    {
        if (*it == toSkip)
            continue;
        std::cout << "Client : " << it->getNickname() << std::endl; 
        if (send(it->getFd(), message.c_str(), message.size(), 0) == -1)
        {
            std::cerr << "Error: Message can't be send to " << it->getNickname() << std::endl;
        }
    }
}
