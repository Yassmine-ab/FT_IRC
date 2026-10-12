#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <list>
#include <map>
#include <set>

class Client; // Déclaration anticipée

class Channel {
public:
    Channel(const std::string& name);
    ~Channel();

    // Membres & opérateurs
    bool addMember(Client* client, const std::string& key = "");
    bool removeMember(Client* client);
    bool isMember(Client* client) const;
    bool addOperator(Client* client);
    bool removeOperator(Client* client);
    bool isOperator(Client* client) const;

    // Modes
    void setMode(char mode, bool enabled, Client* setter = NULL, const std::string& param = "");
    bool getMode(char mode) const;

    // Clé & limite
    void setKey(const std::string& key);
    std::string getKey() const;
    void removeKey();
    void setLimit(int limit);
    int getLimit() const;
    void removeLimit();

    // Topic
    void setTopic(const std::string& topic, Client* setter);
    std::string getTopic() const;

    // Commandes
    bool invite(Client* operatorClient, Client* targetClient);
    bool kick(Client* operatorClient, Client* targetClient, const std::string& reason = "");

    // Utilitaires
    std::string getName() const;
    std::set<Client*> getMembers() const;
    std::set<Client*> getOperators() const;
    std::set<Client*> getInvited() const;

private:
    std::string _name;
    std::set<Client*> _members;
    std::set<Client*> _operators;
    std::set<Client*> _invited;
    std::string _topic;
    std::map<char, bool> _modes; // i, t, k, l, o
    std::string _key;
    int _limit;
};

#endif // CHANNEL_HPP