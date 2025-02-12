#ifndef FT_IRC_HPP
# define FT_IRC_HPP

#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>
#include <cstddef>
#include <cstdlib>
#include <poll.h>

class Server
{
    private:
        const int _socket;

    public:
        Server(void);
        Server(int ip);
        ~Server(void);

        int receive(void) const;
        void read_socket(int clientSocket) const;

};

class Socket
{
    public:

        int     _socket;
        struct sockaddr    *addr;

        Socket(int socket);
        ~Socket(void);
};

#endif