#ifndef SERVER_HPP
#define SERVER_HPP

#include <string>
#include <map>
#include <vector>
#include <poll.h>
#include <stdexcept>

// Forward declarations pour éviter les dépendances circulaires
// Ok si on utilise des pointeurs ou références, pas des objets complets
class Client;
class Channel;
class CommandHandler;

class Server
{
private:
	// Configuration de base
	int					_port;
	std::string			_password;
	std::string			_serverName;
	
	// Socket serveur (listening socket)
	int					_serverSocket;
	
	// Gestion des clients - Map : fd -> Client* pour accès rapide
	std::map<int, Client*>	_clients;
	
	// Gestion des channels - Map : nom du channel -> Channel* pour accès rapide
	std::map<std::string, Channel*>	_channels;
	
	// Surveillance des événements réseau
	std::vector<struct pollfd>	_pollFds; // Liste des sockets à surveiller
	
	// Command handler
	CommandHandler*		_commandHandler;
	
	// Etat du serveur
	bool				_running; // true = serveur actif
	
	// Méthodes privées (utilitaires internes)
	void				_setupServerSocket();
	void				_setNonBlocking(int fd);
	void				_acceptNewClient();
	void				_handleClientData(int fd);
	void				_sendPendingData(int fd);
	void				_disconnectClient(int fd);
	void				_sendToClient(int fd, const std::string& message);
	void				_removePollFd(int fd);
	void				_enablePollOut(int fd);
	void				_disablePollOut(int fd);
	
	// Empêcher la copie
	Server(const Server& other);
	Server& operator=(const Server& other);

public:
	// Constructeur et destructeur
	Server(int port, const std::string& password);
	~Server();
	
	// Méthodes publiques principales
	void				start();
	void				stop();
	void				printStats() const;
	
	// Getters (ne modifient pas l'objet)
	int					getPort() const;
	const std::string&	getPassword() const;
	const std::string&	getServerName() const;
	
	// Gestion des clients
	Client*				getClient(int fd);
	Client*				getClientByNick(const std::string& nickname);
	void				removeClient(int fd);
	
	// Gestion des channels
	Channel*			getChannel(const std::string& name);
	Channel*			createChannel(const std::string& name);
	void				removeChannel(const std::string& name);
	
	// Broadcasting
	void				broadcastToChannel(const std::string& channelName, 
										   const std::string& message, 
										   int excludeFd = -1);
};

#endif
