#include "ft_irc.hpp"

// Server::Server(void) : _socket(socket(AF_INET, SOCK_STREAM, 0))
// {
//     sockaddr_in serverAdress;

//     serverAdress.sin_family = AF_INET;
//     serverAdress.sin_port = htons(8080);
//     serverAdress.sin_addr.s_addr = INADDR_ANY;

//     bind(this->_socket, (struct sockaddr*) &serverAdress, sizeof(serverAdress));

//     if (listen(this->_socket, 5) == -1)
//     {
//         std::cerr << "Error fatal : listen" << std::endl;
//         close(this->_socket);
//     }
// }

// Server::Server(int ip) : _socket(socket(AF_INET, SOCK_STREAM, 0))
// {
//     sockaddr_in serverAdress;

//     serverAdress.sin_family = AF_INET;
//     serverAdress.sin_port = htons(ip);
//     serverAdress.sin_addr.s_addr = INADDR_ANY;

//     bind(this->_socket, (struct sockaddr*) &serverAdress, sizeof(serverAdress));

//     if (listen(this->_socket, 5) == -1)
//     {
//         std::cerr << "Error fatal : listen" << std::endl;
//         close(this->_socket);
//     }
// }

// Server::~Server(void)
// {
//     close(this->_socket);
// }

// int    Server::receive(void) const
// {
//     std::cout << "Waiting message..." << std::endl;

//     int clientSocket = accept(this->_socket, NULL, NULL);

//     if (clientSocket == -1)
//     {
//         std::cerr << "Error fatal : accept" << std::endl;
//         close(this->_socket);
//         return 1;
//     }

//     this->read_socket(clientSocket);

//     close(clientSocket);

//     return 0;
// }

// void Server::read_socket(int clientSocket) const
// {
//     struct pollfd   p;

//     memset(&p, 0, sizeof(p));

//     p.fd = clientSocket;
//     p.events = POLLIN;

//     while (1)
//     {
//         if (poll(&p, 1, 10000) == -1)
//         {
//             std::cerr << "Timeout." << std::endl;
//             return ;
//         }

//         std::cout << p.events << std::endl;
//         std::cout << p.revents << std::endl;

//         char buffer[1024] = {0};
//         recv(clientSocket, buffer, sizeof(buffer), 0);
//         std::cout << "Message from client: " << buffer << std::endl;

//         std::cout << std::endl;

//     }
// }

#include <algorithm>

std::vector<Client> Server::_v;

void	Server::addClient(Client &client)
{
	Server::_v.push_back(client);
}

Client	&Server::getClient(int fd)
{
	std::vector<Client>::iterator	it = Server::_v.begin();

	for (; it != Server::_v.end(); it++)
	{
		if (it->getFd() == fd)
			return *it;
	}
	throw InvalidClientException();
}

Client	&Server::getClient(std::string nickname)
{
	std::vector<Client>::iterator	it = Server::_v.begin();

	for (; it != Server::_v.end(); it++)
	{
		if (!it->getNickname().compare(nickname))
			return *it;
	}
	throw InvalidClientException();
}

const char	*Server::InvalidClientException::what(void) throw()
{
	return ("Invalid Client");
}
