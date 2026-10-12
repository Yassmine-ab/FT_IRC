#include "CommandHandler.hpp"
#include <cctype>

CommandHandler::CommandHandler(Server *server) : _server(server)
{
    _initCommandMap();
}

void CommandHandler::_initCommandMap()
{
    _commandMap["PASS"] = &CommandHandler::cmdPass;
    _commandMap["NICK"] = &CommandHandler::cmdNick;
    _commandMap["USER"] = &CommandHandler::cmdUser;
    
    _commandMap["JOIN"] = &CommandHandler::cmdJoin;
    _commandMap["PART"] = &CommandHandler::cmdPart;
    _commandMap["NAMES"] = &CommandHandler::cmdNames;
    
    _commandMap["PRIVMSG"] = &CommandHandler::cmdPrivmsg;
    _commandMap["NOTICE"] = &CommandHandler::cmdNotice;
    
    _commandMap["KICK"] = &CommandHandler::cmdKick;
    _commandMap["INVITE"] = &CommandHandler::cmdInvite;
    _commandMap["TOPIC"] = &CommandHandler::cmdTopic;
    _commandMap["MODE"] = &CommandHandler::cmdMode;
    
    _commandMap["QUIT"] = &CommandHandler::cmdQuit;
    _commandMap["PING"] = &CommandHandler::cmdPing;
}

void CommandHandler::_parseCommand(const std::string &rawCommand, std::string &command, std::vector<std::string> &params)
{
    params.clear();
    command.clear();
    
    if (rawCommand.empty())
        return;
    
    std::istringstream iss(rawCommand);
    std::string token;
    
    if (!(iss >> command))
        return;
    
    for (size_t i = 0; i < command.length(); ++i)
        command[i] = std::toupper(command[i]);
    
    while (iss >> token)
    {
        if (token[0] == ':')
        {
            std::string trailing = token.substr(1);
            std::string rest;
            std::getline(iss, rest);
            trailing += rest;
            params.push_back(trailing);
            break;
        }
        else
            params.push_back(token);
    }
}

void CommandHandler::handleCommand(Client* client, const std::string &rawCommand)
{
    if (!client || rawCommand.empty())
        return;
    
    std::string command;
    std::vector<std::string> params;
    _parseCommand(rawCommand, command, params);
    
    if (command.empty())
        return;
    
    std::cout << "Command: " << command << " (params: " << params.size() << ") from client " 
              << client->getFd() << std::endl;

    std::map<std::string, CommandFunction>::iterator it = _commandMap.find(command);
    
    if (it != _commandMap.end())
    {
        CommandFunction handler = it->second;
        (this->*handler)(client, params);
    }
    else
    {
        sendNumericReply(client, ERR_UNKNOWNCOMMAND, command + " :Unknown command");
    }
}

void CommandHandler::sendNumericReply(Client* client, const std::string& numeric, const std::string& message)
{
    std::stringstream ss;
    ss << ":" << _server->getServerName() << " " << numeric << " ";
    
    if (!client->getNickname().empty())
        ss << client->getNickname();
    else
        ss << "*";
    
    ss << " " << message << "\r\n";
    
    client->sendMessage(ss.str());
}

void CommandHandler::sendWelcome(Client* client)
{
    std::stringstream ss;
    ss.str("");
    ss << ":Welcome to the Internet Relay Network " << client->getPrefix();
    sendNumericReply(client, RPL_WELCOME, ss.str());
    
    ss.str("");
    ss << ":Your host is " << _server->getServerName() << ", running version 1.0";
    sendNumericReply(client, RPL_YOURHOST, ss.str());

    sendNumericReply(client, RPL_CREATED, ":This server was created today");
    
    ss.str("");
    ss << _server->getServerName() << " 1.0 o itkol";
    sendNumericReply(client, RPL_MYINFO, ss.str());
}

bool CommandHandler::isValidNickname(const std::string& nickname)
{
    if (nickname.empty() || nickname.length() > 9)
        return false;
    
    if (!std::isalpha(nickname[0]))
        return false;
    
    for (size_t i = 0; i < nickname.length(); ++i)
    {
        char c = nickname[i];
        if (!std::isalnum(c) && c != '-' && c != '_' && 
            c != '[' && c != ']' && c != '{' && c != '}' && 
            c != '\\' && c != '|' && c != '^')
            return false;
    }
    
    return true;
}

bool CommandHandler::isValidChannelName(const std::string& name)
{
    if (name.empty() || name.length() > 50)
        return false;
    
    if (name[0] != '#' && name[0] != '&')
        return false;
    
    for (size_t i = 1; i < name.length(); ++i)
    {
        char c = name[i];
        if (std::isspace(c) || c == ',' || c == '\x07')
            return false;
    }
    
    return true;
}
