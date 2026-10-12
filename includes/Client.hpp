#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <string>
#include <vector>
#include <set>

class Client
{
private:
	// Informations de connexion
	int					_fd;
	std::string			_ipAddress;
	int					_port;
	
	// Informations d'identification IRC
	std::string			_nickname;
	std::string			_username;
	std::string			_realname;
	std::string			_hostname;
	
	// État d'authentification
	bool				_authenticated;
	bool				_passwordGiven;
	bool				_registered; // true quand NICK + USER sont donnés
	
	// Buffers pour la communication
	std::string			_recvBuffer;	// Données reçues en attente de traitement
	std::string			_sendBuffer;	// Données à envoyer
	
	// Channels auxquels le client appartient
	std::set<std::string>	_channels;
	
	// Empêcher la copie
	Client(const Client& other);
	Client& operator=(const Client& other);

public:
	// Constructeur et destructeur
	Client(int fd, const std::string& ipAddress, int port);
	~Client();
	
	// Getters
	int					getFd() const;
	std::string			getIpAddress() const;
	int					getPort() const;
	std::string			getNickname() const;
	std::string			getUsername() const;
	std::string			getRealname() const;
	std::string			getHostname() const;
	bool				isAuthenticated() const;
	bool				isPasswordGiven() const;
	bool				isRegistered() const;
	const std::string&	getRecvBuffer() const;
	const std::string&	getSendBuffer() const;
	const std::set<std::string>&	getChannels() const;
	
	// Setters
	void				setNickname(const std::string& nickname);
	void				setUsername(const std::string& username);
	void				setRealname(const std::string& realname);
	void				setHostname(const std::string& hostname);
	void				setAuthenticated(bool authenticated);
	void				setPasswordGiven(bool given);
	void				setRegistered(bool registered);
	
	// Gestion des buffers
	void				appendToRecvBuffer(const char* data, size_t size);
	bool				extractCommand(std::string& command); // Extrait une commande complète (terminée par \r\n)
	void				appendToSendBuffer(const std::string& data);
	void				consumeSendBuffer(size_t bytes); // Retire les premiers bytes du buffer d'envoi
	void				clearSendBuffer();
	
	// Gestion des channels
	void				addChannel(const std::string& channelName);
	void				removeChannel(const std::string& channelName);
	bool				isInChannel(const std::string& channelName) const;
	
	// Utilitaires
	std::string			getPrefix() const; // Retourne le prefix IRC (:nickname!username@hostname)
	void				sendMessage(const std::string& message); // Ajoute au buffer d'envoi
};

#endif
