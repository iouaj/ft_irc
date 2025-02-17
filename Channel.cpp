#include "ft_irc.hpp"

Channel::Channel(Client *admin) : _admin(admin)
{

}

Channel::~Channel(void)
{

}

void    Channel::setAdmin(Client *admin)
{
    Client *prev_admin = this->_admin;

    this->_admin = admin;

    send_priv(*prev_admin, prev_admin->getNickname() + ", you are no longer operator.\n");

    send_priv(*admin, admin->getNickname() + ", you are now operator.");
}

Client    *Channel::getAdmin(void) const
{
    return this->_admin;
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