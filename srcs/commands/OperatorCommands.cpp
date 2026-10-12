#include "CommandHandler.hpp"
#include "Channel.hpp"
#include <sstream>
#include <cstdlib>

void CommandHandler::cmdKick(Client* client, const std::vector<std::string> &params)
{
    if (!client->isRegistered())
        return;
    
    if (params.size() < 2)
    {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, "KICK :Not enough parameters");
        return;
    }
    
    const std::string& channelName = params[0];
    const std::string& targetNick = params[1];
    std::string reason = "Kicked";
    if (params.size() > 2)
        reason = params[2];
    
    Channel* chan = _server->getChannel(channelName);
    if (!chan)
    {
        sendNumericReply(client, ERR_NOSUCHCHANNEL, channelName + " :No such channel");
        return;
    }
    
    if (!chan->isOperator(client))
    {
        sendNumericReply(client, ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
        return;
    }
    
    Client* targetClient = _server->getClientByNick(targetNick);
    if (!targetClient)
    {
        sendNumericReply(client, ERR_NOSUCHNICK, targetNick + " :No such nick/channel");
        return;
    }
    
    if (!chan->isMember(targetClient))
    {
        sendNumericReply(client, ERR_USERNOTINCHANNEL, targetNick + " " + channelName + " :They aren't on that channel");
        return;
    }
    
    std::string kickMsg = client->getPrefix() + " KICK " + channelName + " " + targetNick + " :" + reason + "\r\n";
    _server->broadcastToChannel(channelName, kickMsg, -1);
    
    chan->removeMember(targetClient);
    chan->removeOperator(targetClient);
    targetClient->removeChannel(channelName);
    
    std::cout << targetNick << " kicked from " << channelName << " by " << client->getNickname() << std::endl;
}

void CommandHandler::cmdInvite(Client* client, const std::vector<std::string> &params)
{
    if (!client->isRegistered())
        return;
    
    if (params.size() < 2)
    {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, "INVITE :Not enough parameters");
        return;
    }
    
    const std::string& targetNick = params[0];
    const std::string& channelName = params[1];
    
    Channel* chan = _server->getChannel(channelName);
    if (!chan)
    {
        sendNumericReply(client, ERR_NOSUCHCHANNEL, channelName + " :No such channel");
        return;
    }
    
    if (!chan->isMember(client))
    {
        sendNumericReply(client, ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
        return;
    }
    
    if (chan->getMode('i') && !chan->isOperator(client))
    {
        sendNumericReply(client, ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
        return;
    }
    
    Client* targetClient = _server->getClientByNick(targetNick);
    if (!targetClient)
    {
        sendNumericReply(client, ERR_NOSUCHNICK, targetNick + " :No such nick/channel");
        return;
    }
    
    if (chan->isMember(targetClient))
    {
        sendNumericReply(client, ERR_USERONCHANNEL, targetNick + " " + channelName + " :is already on channel");
        return;
    }
    
    chan->invite(client, targetClient);
    
    sendNumericReply(client, RPL_INVITING, targetNick + " " + channelName);
    
    std::string inviteMsg = client->getPrefix() + " INVITE " + targetNick + " " + channelName + "\r\n";
    targetClient->sendMessage(inviteMsg);
    
    std::cout << "Invite: " << targetNick << " invited to " << channelName << " by " << client->getNickname() << std::endl;
}

void CommandHandler::cmdTopic(Client* client, const std::vector<std::string> &params)
{
    if (!client->isRegistered())
        return;
    
    if (params.empty())
    {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, "TOPIC :Not enough parameters");
        return;
    }
    
    const std::string& channelName = params[0];
    
    Channel* chan = _server->getChannel(channelName);
    if (!chan)
    {
        sendNumericReply(client, ERR_NOSUCHCHANNEL, channelName + " :No such channel");
        return;
    }

    if (!chan->isMember(client))
    {
        sendNumericReply(client, ERR_NOTONCHANNEL, channelName + " :You're not on that channel");
        return;
    }
    
    if (params.size() == 1)
    {
        std::string topic = chan->getTopic();
        if (topic.empty())
            sendNumericReply(client, RPL_NOTOPIC, channelName + " :No topic is set");
        else
            sendNumericReply(client, RPL_TOPIC, channelName + " :" + topic);
        return;
    }
    
    const std::string& newTopic = params[1];
    
    if (chan->getMode('t') && !chan->isOperator(client))
    {
        sendNumericReply(client, ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
        return;
    }
    
    chan->setTopic(newTopic, client);
    
    std::string topicMsg = client->getPrefix() + " TOPIC " + channelName + " :" + newTopic + "\r\n";
    _server->broadcastToChannel(channelName, topicMsg, -1);
    
    std::cout << "Topic of " << channelName << " changed by " << client->getNickname() << ": " << newTopic << std::endl;
}

void CommandHandler::cmdMode(Client* client, const std::vector<std::string> &params)
{
    if (!client->isRegistered())
        return;
    
    if (params.empty())
    {
        sendNumericReply(client, ERR_NEEDMOREPARAMS, "MODE :Not enough parameters");
        return;
    }
    
    const std::string& channelName = params[0];
    
    Channel* chan = _server->getChannel(channelName);
    if (!chan)
    {
        sendNumericReply(client, ERR_NOSUCHCHANNEL, channelName + " :No such channel");
        return;
    }
    
    if (params.size() == 1)
    {
        std::string modeStr = "+";
        if (chan->getMode('i')) modeStr += "i";
        if (chan->getMode('t')) modeStr += "t";
        if (chan->getMode('k')) modeStr += "k";
        if (chan->getMode('l')) modeStr += "l";
        
        sendNumericReply(client, RPL_CHANNELMODEIS, channelName + " " + modeStr);
        return;
    }

    if (!chan->isOperator(client))
    {
        sendNumericReply(client, ERR_CHANOPRIVSNEEDED, channelName + " :You're not channel operator");
        return;
    }
    
    const std::string& modeString = params[1];
    bool adding = true;
    size_t paramIndex = 2;
    
    std::string appliedModes;
    std::string appliedParams;
    
    for (size_t i = 0; i < modeString.length(); ++i)
    {
        char mode = modeString[i];
        
        if (mode == '+')
        {
            adding = true;
            continue;
        }
        else if (mode == '-')
        {
            adding = false;
            continue;
        }
        
        if (mode == 'i' || mode == 't')
        {
            chan->setMode(mode, adding, client);
            appliedModes += (adding ? '+' : '-');
            appliedModes += mode;
        }
        else if (mode == 'k')
        {
            if (adding)
            {
                if (paramIndex < params.size())
                {
                    const std::string& key = params[paramIndex++];
                    chan->setMode('k', true, client, key);
                    appliedModes += "+k";
                    appliedParams += " " + key;
                }
            }
            else
            {
                chan->setMode('k', false, client);
                appliedModes += "-k";
            }
        }
        else if (mode == 'l')
        {
            if (adding)
            {
                if (paramIndex < params.size())
                {
                    const std::string& limitStr = params[paramIndex++];
                    chan->setMode('l', true, client, limitStr);
                    appliedModes += "+l";
                    appliedParams += " " + limitStr;
                }
            }
            else
            {
                chan->setMode('l', false, client);
                appliedModes += "-l";
            }
        }
        else if (mode == 'o')
        {
            if (paramIndex < params.size())
            {
                const std::string& targetNick = params[paramIndex++];
                Client* targetClient = _server->getClientByNick(targetNick);
                
                if (!targetClient)
                {
                    sendNumericReply(client, ERR_NOSUCHNICK, targetNick + " :No such nick/channel");
                    continue;
                }
                
                if (!chan->isMember(targetClient))
                {
                    sendNumericReply(client, ERR_USERNOTINCHANNEL, targetNick + " " + channelName + " :They aren't on that channel");
                    continue;
                }
                
                if (adding)
                    chan->addOperator(targetClient);
                else
                    chan->removeOperator(targetClient);
                
                appliedModes += (adding ? "+o" : "-o");
                appliedParams += " " + targetNick;
            }
        }
        else
        {
            sendNumericReply(client, ERR_UNKNOWNMODE, std::string(1, mode) + " :is unknown mode char to me");
        }
    }

    if (!appliedModes.empty())
    {
        std::string modeMsg = client->getPrefix() + " MODE " + channelName + " " + appliedModes + appliedParams + "\r\n";
        _server->broadcastToChannel(channelName, modeMsg, -1);
        
        std::cout << "Mode changed on " << channelName << ": " << appliedModes << appliedParams << std::endl;
    }
}
