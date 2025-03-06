#include "Channel.hpp"

Channel::Channel(Client *op, std::string name) : _op(op), _name(name)
{
    this->_invite_only = false;
}

Channel::~Channel(void)
{

}

void    Channel::setOp(Client *op)
{
    Client *prev_op = this->_op;

    this->_op = op;

    send_priv(*prev_op, prev_op->getNickname() + ", you are no longer operator.");

    send_priv(*op, op->getNickname() + ", you are now operator.");
}

Client    *Channel::getOp(void) const
{
    return this->_op;
}

const std::list<Client>   &Channel::getClients(void) const
{
    const std::list<Client> &ref = this->_clients;
    return ref;
}

void    Channel::addClient(Client &client)
{
    this->_clients.push_back(client);
}

void    Channel::removeClient(Client &client)
{
    this->_clients.remove(client);

    if (this->_clients.size() == 0)
        Server::deleteChannel(*this);
}

void    Channel::setName(std::string name)
{
    this->_name = name;
}

const std::string   &Channel::getName(void) const
{
    const std::string &name = this->_name;
    return name;
}

bool    Channel::isInviteOnly(void) const
{
    return this->_invite_only == true;
}

bool    Channel::haveClient(const Client &client) const
{
    std::list<Client>::const_iterator it = this->_clients.begin();

    for (; it != this->_clients.end(); it++) {
        if (*it == client)
            return true;
    }
    return false;
}

bool    Channel::operator==(const Channel &channel) const
{
    return this->_name == channel.getName();
}

void    Channel::broadcast(std::string message) const
{
    std::list<Client>::const_iterator it = this->_clients.begin();

    message = message + "\r\n";
    for (; it != this->_clients.end(); it++)
    {
        send(it->getFd(), message.c_str(), message.size(), 0);
    }
}

void    Channel::broadcast(std::string message, const Client &exclude) const
{
    std::list<Client>::const_iterator it = this->_clients.begin();

    message = message + "\r\n";
    for (; it != this->_clients.end(); it++)
    {
        if (*it == exclude)
            continue;
        send(it->getFd(), message.c_str(), message.size(), 0);
    }
}

void    Channel::kickMember(const Client &exec, const Client &target, std::string reason)
{
    if (*this->_op != exec) {
        send_error(exec, ERR_CHANOPRIVSNEEDED, this->_name, ":You're not channel operator");
        return ;
    }

    for (std::list<Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); it++)
    {
        if (*it == target) {
            std::string msg = ":" + exec.getNickname() + "!" + exec.getUsername() + "@localhost" + " KICK " + this->_name + " " + target.getNickname() + " " + reason;
            this->broadcast(msg);
            this->_clients.remove(target);
            return ;
        }
    }
}