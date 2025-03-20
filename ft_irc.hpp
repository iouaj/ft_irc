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
#include <signal.h>

#include "Client.hpp"
#include "Channel.hpp"
#include "Server.hpp"
#include "Request.hpp"

#define SERVER_NAME	"localhost"

#define MAX_LIMIT_MEMBER_CHANNEL 9999

#define ERR_NOSUCHNICK(client, target_nick) ":localhost 401 " + std::string(client) + " " + std::string(target_nick) + " :No such nick/channel\r\n"
#define ERR_NOSUCHCHANNEL(client, channel) ":localhost 403 " + std::string(client) + " " + std::string(channel) + " :No such channel\r\n"
#define ERR_CANNOTSENDTOCHAN(client, channel) ":localhost 404 " + std::string(client) + " " + std::string(channel) + " :Cannot send to channel\r\n"
#define ERR_NOTEXTTOSEND(client) ":localhost 412 " + std::string(client) + " :No text to send\r\n"
#define ERR_INPUTTOOLONG(client) ":localhost 414 " + std::string(client) + " :Input line too long (max 510 bytes)\r\n"
#define ERR_UNKNOWNCOMMAND(client, command) ":localhost 421 " + std::string(client) + " " + std::string(command) + " :Unknown command\r\n"
#define ERR_ERRONEUSNICKNAME(client, nickname) ":localhost 432 " + std::string(client) + " " + std::string(nickname) + " :Erroneous nickname\r\n"
#define ERR_NICKNAMEINUSE(client, nickname) ":localhost 433 " + std::string(client) + " " + std::string(nickname) + " :Nickname is already in use\r\n"
#define ERR_USERNOTINCHANNEL(client, target, channel) ":localhost 441 " + std::string(client) + " " + std::string(target) + " " + std::string(channel) + " :They aren't on that channel\r\n"
#define ERR_NOTONCHANNEL(client, channel) ":localhost 442 " + std::string(client) + " " + std::string(channel) + " :You're not on that channel\r\n"
#define ERR_USERONCHANNEL(client, channel) ":localhost 443 " + std::string(client) + " " + std::string(client) + " " + std::string(channel) + " :User is already on channel\r\n"
#define ERR_NOTREGISTERED ":localhost 451 * :You have not registered\r\n"
#define ERR_NEEDMOREPARAM(client, command) ":localhost 461 " + std::string(client) + " " + std::string(command) + " :Not enough parameters\r\n"
#define ERR_ALREADYREGISTRED(client) ":localhost 462 " + std::string(client) + " :You may not reregister\r\n"
#define ERR_CHANNELISFULL(client, channel) ":localhost 471 " + std::string(client) + " " + std::string(channel) + " :Cannot join channel(+l)\r\n"
#define ERR_UNKNOWNMODE(client, modechar) ":localhost 472 " + std::string(client) + " " + std::string(modechar) + " :is unknown mode char to me\r\n"
#define ERR_INVITEONLYCHAN(client, channel) ":localhost 473 " + std::string(client) + " " + std::string(channel) + " :Cannot join channel (+i)\r\n"
#define ERR_BANNEDFROMCHAN(client, channel) ":localhost 474 " + std::string(client) + " " + std::string(channel) + " :Cannot join channel (+b)\r\n"
#define ERR_BADCHANNELKEY(client, channel) ":localhost 475 " + std::string(client) + " " + std::string(channel) + " :Cannot join channel (+k)\r\n"
#define ERR_CHANOPRIVSNEEDED(client, channel) ":localhost 482 " + std::string(client) + " " + std::string(channel) + " :You're not channel operator\r\n"
#define ERR_PASSWDMISMATCH(client) ":localhost 464 " + std::string(client) + " :Password incorrect\r\n"
#define ERR_USERSDONTMATCH(client) ":localhost 502 " + std::string(client) + " :Users don't match\r\n"
#define ERR_INVALIDMODEPARAM(client, channel, param) ":localhost 696 " + std::string(client) + " " + std::string(channel) + " " + std::string(param) + " :Invalid parameter for mode\r\n"

#define RPL_WELCOME(client) ":localhost 001 " + std::string(client) + " :Welcome !\r\n"
#define RPL_INVITING(client, channel, target) ":localhost 341 " + std::string(client) + " " + std::string(target) + " " + std::string(channel) + "\r\n"
#define RPL_NOTOPIC(client, channel) ":localhost 331 " + std::string(client) + " " + std::string(channel) + " :No topic is set\r\n"
#define RPL_TOPIC(client, channel, topic) ":localhost 332 " + std::string(client) + " " + std::string(channel) + " :" + std::string(topic) + "\r\n"
#define RPL_ENDOFNAMES(client, channel) ":localhost 366 " + std::string(client) + " " + std::string(channel) + " :End of /NAMES list\r\n"
#define RPL_CHANNELMODEIS(client, channel, mode) ":localhost 324 " + std::string(client) + " " + std::string(channel) + " " + std::string(mode) + "\r\n"

#define RPL_CLOSE_CONNEXION(client) ":localhost ERROR :Closing Link: " + std::string(client) + " (Connection reset by peer)\r\n"
#define RPL_PONG(token) "PONG " + std::string(token) + "\r\n"

template <typename T> std::string toStr(T tmp)
{
	std::ostringstream out;
	out << tmp;
	return out.str();
}

void	sendServer(const Client &target, std::string message);

void	sendNameReply(const Client &target, const Channel &channel);

int	setSignal(void);

std::string	clean_string(std::string str);
bool		isNumericString(const std::string &str);
#endif
