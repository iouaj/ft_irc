#include "ft_irc.hpp"

void set_nonblocking(int fd) {
	fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) | O_NONBLOCK);
}

int main (int argc, char *argv[])
{
	if (argc != 2)
	{
		std::cerr << "Error: Missing arguments" << std::endl;
		return 1;
	}

	(void) argv;

	int socketServer = socket(AF_INET, SOCK_STREAM, 0);
	if (socketServer == -1)
	{
		std::cerr << "Error: socket" << std::endl;
		return 1;
	}

	struct sockaddr_in	server_addr;

	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = INADDR_ANY;
	server_addr.sin_port = htons(8080);

	if (bind(socketServer, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
	{
		std::cerr << "Error: bind socket" << std::endl;
		return 1;
	}

	if (listen(socketServer, 10) == -1)
	{
		std::cerr << "Error: listen" << std::endl;
		return 1;
	}

	set_nonblocking(socketServer);

	int	epoll_fd = epoll_create1(0);
	if (epoll_fd == -1)
	{
		std::cerr << "Error: epoll" << std::endl;
		return 1;
	}

	struct epoll_event event;

	event.events = EPOLLIN;
	event.data.fd = socketServer;

	epoll_ctl(epoll_fd, EPOLL_CTL_ADD, socketServer, &event);

	struct epoll_event events[10];

	std::cout << "Serveur en écoute sur le port " << 8080 << "..." << std::endl;

	std::cout << "Socker Server " << socketServer << std::endl;

	while (true)
	{

		int num_events = epoll_wait(epoll_fd, events, 10, -1);

		for (int i = 0; i < num_events; i++) {
			if (events[i].data.fd == socketServer) {
				// 🆕 Nouveau client

				sockaddr_in client_addr;
				socklen_t client_len = sizeof(client_addr);

				int client_fd = accept(socketServer, (struct sockaddr *)&client_addr, &client_len);
				if (client_fd != -1) {
					event.events = EPOLLIN;
					event.data.fd = client_fd;
					epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);

					std::cout << "Nouvelle connexion acceptée." << std::endl;

					Client	client("none", client_fd);

					Server::addClient(client);

					send(client_fd, "Set a nickname\n", 15, 0);
				}
			} else {
				// 📩 Message reçu d'un client
				char buffer[1024];
				int bytes_read = read(events[i].data.fd, buffer, sizeof(buffer));
				if (bytes_read > 0) {
					// send(events[i].data.fd, buffer, bytes_read, 0);  // Echo

					try
					{
						buffer[bytes_read] = 0;
						Request req(buffer);
						req.print();

						req.exec(events[i].data.fd);
					}
					catch(const Request::InvalidRequestException& e)
					{
						std::cerr << e.what() << std::endl;
					}

				} else {
					// 🔌 Déconnexion du client
					close(events[i].data.fd);
				}
			}
		}

	}
}
