/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   CommandHandler.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yaabdall <yaabdall@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/06 14:09:59 by vgalmich          #+#    #+#             */
/*   Updated: 2025/11/14 19:09:45 by yaabdall         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#pragma once

#include "Channel.hpp"
#include "Server.hpp"
#include "Client.hpp"
#include "NumericReplies.hpp"
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <sstream>

// structure de base de la classe CommandHandler
class CommandHandler
{
    public:
        CommandHandler(Server *server);
        
        // typedef pour pointeur vers fonctions membres de CommandHandler
        typedef void (CommandHandler::*CommandFunction)(Client* client, const std::vector<std::string>&);
        
        void handleCommand(Client* client, const std::string &rawCommand);
        
    private:
        Server *_server; // lien vers le serveur
        
        // Map des commandes vers leurs handlers
        std::map<std::string, CommandFunction> _commandMap;
        
        // Initialiser la map
        void _initCommandMap();
        
        // Parse une commande IRC brute en commande + params
        void _parseCommand(const std::string &rawCommand, std::string &command, std::vector<std::string> &params);
        
        // Commandes d'authentification
        void cmdPass(Client* client, const std::vector<std::string> &params);
        void cmdNick(Client* client, const std::vector<std::string> &params);
        void cmdUser(Client* client, const std::vector<std::string> &params);
        
        // Commandes de channel
        void cmdJoin(Client* client, const std::vector<std::string> &params);
        void cmdPart(Client* client, const std::vector<std::string> &params);
        void cmdNames(Client* client, const std::vector<std::string> &params);
        
        // Commandes de messages
        void cmdPrivmsg(Client* client, const std::vector<std::string> &params);
        void cmdNotice(Client* client, const std::vector<std::string> &params);
        
        // Commandes d'opérateur
        void cmdKick(Client* client, const std::vector<std::string> &params);
        void cmdInvite(Client* client, const std::vector<std::string> &params);
        void cmdTopic(Client* client, const std::vector<std::string> &params);
        void cmdMode(Client* client, const std::vector<std::string> &params);
        
        // Commandes utilitaires
        void cmdQuit(Client* client, const std::vector<std::string> &params);
        void cmdPing(Client* client, const std::vector<std::string> &params);
        
        // Helpers
        void sendWelcome(Client* client);
        void sendNumericReply(Client* client, const std::string& numeric, const std::string& message);
        bool isValidNickname(const std::string& nickname);
        bool isValidChannelName(const std::string& name);
};