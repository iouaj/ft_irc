#include "ft_irc.hpp"

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

    send_priv(*prev_op, prev_op->getNickname() + ", you are no longer operator.\n");

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

const std::string &Channel::getPermsChannel(void) const
{
    const std::string &perms = this->_perms_channel;
    return perms;
}

void    Channel::setPermsChannel(std::string perm)
{
    this->_perms_channel = perm;
}

bool    Channel::isInviteOnly(void) const
{
    return this->_invite_only == true;
}

bool    Channel::operator==(const Channel &channel) const
{
    return this->_name == channel.getName();
}

void    Channel::kickMember(const Client &exec, const Client &target, std::string reason)
{
    if (*this->_op != exec) {
        send_priv(exec, ":" + toStr(SERVER_NAME) + " " + toStr(ERR_CHANOPRIVSNEEDED) + " " + this->_name + " :You're not channel operator");
        return ;
    }

    for (std::list<Client>::iterator it = this->_clients.begin(); it != this->_clients.end(); it++)
    {
        if (*it == target)
        {
            this->_clients.remove(target);
            send_group(this->_clients, "KICK " + this->_name + " " + target.getNickname() + (reason.empty() ? "" : " " + reason ), *this->_clients.end());
        }
    }
}