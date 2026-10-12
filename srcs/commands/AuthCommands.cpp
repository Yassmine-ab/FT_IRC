#include "CommandHandler.hpp"

void CommandHandler::cmdPass(Client* client, const std::vector<std::string> &params)
{
    if (client->isRegistered())
    {
        sendNumericReply(client, ERR_ALREADYREGISTRED, ":You may not reregister");
        return;
    }
    
    if (params.empty())
    {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, "PASS :Not enough parameters");
        return;
    }
    
    const std::string& password = params[0];
    if (password == _server->getPassword())
    {
        client->setPasswordGiven(true);
        std::cout << "Client " << client->getFd() << " provided correct password" << std::endl;
    }
    else
    {
        sendNumericReply(client, ERR_PASSWDMISMATCH, ":Password incorrect");
        std::cout << "Client " << client->getFd() << " provided wrong password" << std::endl;
    }
}

void CommandHandler::cmdNick(Client* client, const std::vector<std::string> &params)
{
    if (params.empty())
    {
        sendNumericReply(client, ERR_NONICKNAMEGIVEN, ":No nickname given");
        return;
    }
    
    const std::string& newNick = params[0];
    
    if (!isValidNickname(newNick))
    {
        sendNumericReply(client, ERR_ERRONEUSNICKNAME, newNick + " :Erroneous nickname");
        return;
    }
    
    Client* existingClient = _server->getClientByNick(newNick);
    if (existingClient && existingClient != client)
    {
        sendNumericReply(client, ERR_NICKNAMEINUSE, newNick + " :Nickname is already in use");
        return;
    }
    
    std::string oldNick = client->getNickname();
    
    if (client->isRegistered() && !oldNick.empty())
    {
        const std::set<std::string>& channels = client->getChannels();
        for (std::set<std::string>::const_iterator it = channels.begin(); it != channels.end(); ++it)
        {
            Channel* channel = _server->getChannel(*it);
            if (channel)
            {
                std::string nickChange = client->getPrefix() + " NICK :" + newNick + "\r\n";
                _server->broadcastToChannel(*it, nickChange, -1);
            }
        }
    }
    
    client->setNickname(newNick);
    
    if (!client->getUsername().empty() && client->isPasswordGiven() && !client->isRegistered())
    {
        client->setRegistered(true);
        sendWelcome(client);
    }
}

void CommandHandler::cmdUser(Client* client, const std::vector<std::string> &params)
{
    if (client->isRegistered())
    {
        sendNumericReply(client, ERR_ALREADYREGISTRED, ":You may not reregister");
        return;
    }
    
    if (params.size() < 4)
    {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, "USER :Not enough parameters");
        return;
    }
    
    if (!client->isPasswordGiven())
    {
        sendNumericReply(client, ERR_PASSWDMISMATCH, ":You must send PASS first");
        return;
    }
    
    const std::string& username = params[0];
    const std::string& realname = params[3];
    
    client->setUsername(username);
    client->setRealname(realname);
    
    if (!client->getNickname().empty())
    {
        client->setRegistered(true);
        sendWelcome(client);
    }
}

void CommandHandler::cmdQuit(Client* client, const std::vector<std::string> &params)
{
    std::string reason = "Client quit";
    if (!params.empty())
        reason = params[0];
    
    std::cout << "Client " << client->getNickname() << " quit: " << reason << std::endl;
    
    const std::set<std::string>& channels = client->getChannels();
    for (std::set<std::string>::const_iterator it = channels.begin(); it != channels.end(); ++it)
    {
        Channel* channel = _server->getChannel(*it);
        if (channel)
        {
            std::string quitMsg = client->getPrefix() + " QUIT :" + reason + "\r\n";
            _server->broadcastToChannel(*it, quitMsg, client->getFd());
            channel->removeMember(client);
            channel->removeOperator(client);
        }
    }
}

void CommandHandler::cmdPing(Client* client, const std::vector<std::string> &params)
{
    if (params.empty())
    {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, "PING :Not enough parameters");
        return;
    }
    
    std::string response = ":" + _server->getServerName() + " PONG " + _server->getServerName() + " :" + params[0] + "\r\n";
    client->sendMessage(response);
}
