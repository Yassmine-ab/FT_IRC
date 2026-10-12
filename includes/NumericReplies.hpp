/* ************************************************************************** */
/*                                                                            */
/*                                                        :::      ::::::::   */
/*   NumericReplies.hpp                                 :+:      :+:    :+:   */
/*                                                    +:+ +:+         +:+     */
/*   By: yaabdall <yaabdall@student.42.fr>          +#+  +:+       +#+        */
/*                                                +#+#+#+#+#+   +#+           */
/*   Created: 2025/11/08 18:05:40 by vgalmich          #+#    #+#             */
/*   Updated: 2025/11/14 19:09:45 by yaabdall         ###   ########.fr       */
/*                                                                            */
/* ************************************************************************** */

#ifndef NUMERIC_REPLIES_HPP
#define NUMERIC_REPLIES_HPP

// Success replies (2xx-3xx)
#define RPL_WELCOME            "001"  // :server 001 nick :Welcome to the Internet Relay Network nick!user@host
#define RPL_YOURHOST           "002"  // :server 002 nick :Your host is <servername>, running version <ver>
#define RPL_CREATED            "003"  // :server 003 nick :This server was created <date>
#define RPL_MYINFO             "004"  // :server 004 nick <servername> <version> <available user modes> <available channel modes>
#define RPL_NOTOPIC            "331"  // :server 331 nick #channel :No topic is set
#define RPL_TOPIC              "332"  // :server 332 nick #channel :topic text
#define RPL_INVITING           "341"  // :server 341 nick invited_nick #channel
#define RPL_NAMREPLY           "353"  // :server 353 nick = #channel :nick1 nick2 nick3
#define RPL_ENDOFNAMES         "366"  // :server 366 nick #channel :End of /NAMES list
#define RPL_CHANNELMODEIS      "324"  // :server 324 nick #channel +modes [mode_params]

// Error replies (4xx-5xx)
#define ERR_NOSUCHNICK         "401"  // :server 401 nick target :No such nick/channel
#define ERR_NOSUCHSERVER       "402"  // :server 402 nick server :No such server
#define ERR_NOSUCHCHANNEL      "403"  // :server 403 nick #channel :No such channel
#define ERR_CANNOTSENDTOCHAN   "404"  // :server 404 nick #channel :Cannot send to channel
#define ERR_TOOMANYCHANNELS    "405"  // :server 405 nick #channel :You have joined too many channels
#define ERR_NOTEXTTOSEND       "412"  // :server 412 nick :No text to send
#define ERR_UNKNOWNCOMMAND     "421"  // :server 421 nick command :Unknown command
#define ERR_NONICKNAMEGIVEN    "431"  // :server 431 nick :No nickname given
#define ERR_ERRONEUSNICKNAME   "432"  // :server 432 nick nickname :Erroneous nickname
#define ERR_NICKNAMEINUSE      "433"  // :server 433 nick nickname :Nickname is already in use
#define ERR_USERNOTINCHANNEL   "441"  // :server 441 nick target #channel :They aren't on that channel
#define ERR_NOTONCHANNEL       "442"  // :server 442 nick #channel :You're not on that channel
#define ERR_USERONCHANNEL      "443"  // :server 443 nick user #channel :is already on channel
#define ERR_NEEDMOREPARAMS     "461"  // :server 461 nick command :Not enough parameters
#define ERR_ALREADYREGISTRED   "462"  // :server 462 nick :You may not reregister
#define ERR_PASSWDMISMATCH     "464"  // :server 464 nick :Password incorrect
#define ERR_CHANNELISFULL      "471"  // :server 471 nick #channel :Cannot join channel (+l)
#define ERR_UNKNOWNMODE        "472"  // :server 472 nick mode :is unknown mode char to me
#define ERR_INVITEONLYCHAN     "473"  // :server 473 nick #channel :Cannot join channel (+i)
#define ERR_BADCHANNELKEY      "475"  // :server 475 nick #channel :Cannot join channel (+k)
#define ERR_CHANOPRIVSNEEDED   "482"  // :server 482 nick #channel :You're not channel operator

#endif