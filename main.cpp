#include "ft_irc.hpp"

int main(int argc, char *argv[])
{

    if (argc != 2)
    {
        std::cerr << "Missing arguments" << std::endl;
        return 1;
    }

    Server  s(std::atoi(argv[1]));
    
    s.receive();

    return 0;
}