#ifndef FT_IRC_HPP
# define FT_IRC_HPP

#include <cstring>
#include <string>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstddef>
#include <cstdlib>
#include <poll.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <map>
#include <sstream>
#include <vector>
#include <list>
#include <algorithm>

#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"
#include "Request.hpp"

#define SERVER_NAME	"localhost"

#define ERR_NOISSUE 001
#define RPL_ENDOFWHO 315
#define ERR_NOSUCHNICK 401
#define ERR_NOSUCHCHANNEL 403
#define ERR_CANNOTSENDTOCHAN 404
#define ERR_NOTEXTTOSEND 412
#define ERR_NOTONCHANNEL 442
#define ERR_NEEDMOREPARAMS 461
#define ERR_INVITEONLYCHAN 473
#define ERR_CHANOPRIVSNEEDED 482
#define ERR_USERSDONTMATCH 502

template <typename T> std::string toStr(T tmp)
{
    std::ostringstream out;
    out << tmp;
    return out.str();
}

void    send_priv(const Client &client, std::string message);
void    send_group(const std::list<Client> &clients, std::string message, const Client &toSkip);
void    send_error(const Client &client, int error, std::string arg, std::string msg);
#endif
