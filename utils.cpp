#include "ft_irc.hpp"

void	sendServer(const Client &target, std::string message)
{
	if (send(target.getFd(), message.c_str(), message.size(), 0) == -1) {
		std::cerr << "Error: Message can't be send" << std::endl;
	}
}

std::string	clean_string(std::string str)
{
	std::string clean;
	for (std::size_t i = 0; i <= str.size(); i++)
	{
		if (str[i] != '\n' && str[i] != '\r' && str[i] != 0) {
			clean += str[i];
		}
	}
	return clean;
}

bool	isNumericString(const std::string &str)
{
	if (str.empty() || str.size() > 5) return false;

	for (size_t i = 0; i < str.size(); i++)
	{
		if (i == 0 && (str[i] == '-' || str[i] == '+')) continue;

		if (std::isdigit(str[i]) == false) return false;
	}

	return true;
}

void	sendNameReply(const Client &target, const Channel &channel)
{
	const std::string	&target_name = target.getNickname();
	const std::string	&channel_name = channel.getName();
	const std::string symbol = channel.getSymbol();
	std::list<std::string> list = channel.getList();

	while (list.empty() == false)
	{
		std::string	msg = ":localhost 353 " + target_name + " " + symbol + " " + channel_name + " :";

		while (list.empty() == false && msg.size() < 510)
		{
			msg.append(list.front() + " ");
			list.pop_front();
		}
		msg += "\r\n";
		sendServer(target, msg);
	}
	sendServer(target, RPL_ENDOFNAMES(target_name, channel_name));
}

void	handle_sigint(int sig)
{
	(void) sig;
	Server::shutdown_server();
	// Server::clearFds();
	// close(Server::getSocket());
	// close(Server::getEpolFd());
	exit(EXIT_SUCCESS);
}

int	setSignal(void)
{
	struct sigaction	sa;

	sa.sa_flags = 0;
	sa.sa_handler = &handle_sigint;
	if (sigemptyset(&sa.sa_mask) == -1)
		return 0;
	if (sigaction(SIGINT, &sa, NULL) == -1)
		return 0;
	return 1;
}
