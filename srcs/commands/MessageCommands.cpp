#include "CommandHandler.hpp"

void CommandHandler::cmdPrivmsg(Client* client, const std::vector<std::string> &params)
{
    if (!client->isRegistered())
        return;
    
    if (params.size() < 2)
    {
        if (params.empty())
            sendNumericReply(client, ERR_NEEDMOREPARAMS, "PRIVMSG :Not enough parameters");
        else
            sendNumericReply(client, ERR_NOTEXTTOSEND, ":No text to send");
        return;
    }
    
    const std::string& target = params[0];
    const std::string& message = params[1];
    
    if (message.empty())
    {
        sendNumericReply(client, ERR_NOTEXTTOSEND, ":No text to send");
        return;
    }
    
    if (target[0] == '#' || target[0] == '&')
    {
        Channel* chan = _server->getChannel(target);
        if (!chan)
        {
            sendNumericReply(client, ERR_NOSUCHCHANNEL, target + " :No such channel");
            return;
        }
        
        if (!chan->isMember(client))
        {
            sendNumericReply(client, ERR_CANNOTSENDTOCHAN, target + " :Cannot send to channel");
            return;
        }
        
        std::string msg = client->getPrefix() + " PRIVMSG " + target + " :" + message + "\r\n";
        _server->broadcastToChannel(target, msg, client->getFd());
    }
    else
    {
        Client* targetClient = _server->getClientByNick(target);
        if (!targetClient)
        {
            sendNumericReply(client, ERR_NOSUCHNICK, target + " :No such nick/channel");
            return;
        }
        
        std::string msg = client->getPrefix() + " PRIVMSG " + target + " :" + message + "\r\n";
        targetClient->sendMessage(msg);
    }
}

void CommandHandler::cmdNotice(Client* client, const std::vector<std::string> &params)
{
    if (!client->isRegistered())
        return;
    
    if (params.size() < 2)
        return;
    
    const std::string& target = params[0];
    const std::string& message = params[1];
    
    if (message.empty())
        return;
    
    if (target[0] == '#' || target[0] == '&')
    {
        Channel* chan = _server->getChannel(target);
        if (!chan || !chan->isMember(client))
            return;
        
        std::string msg = client->getPrefix() + " NOTICE " + target + " :" + message + "\r\n";
        _server->broadcastToChannel(target, msg, client->getFd());
    }
    else
    {
        Client* targetClient = _server->getClientByNick(target);
        if (!targetClient)
            return;
        
        std::string msg = client->getPrefix() + " NOTICE " + target + " :" + message + "\r\n";
        targetClient->sendMessage(msg);
    }
}
