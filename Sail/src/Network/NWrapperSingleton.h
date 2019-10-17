#pragma once

#include "NWrapperHost.h"
#include "NWrapperClient.h"

class NetworkSenderSystem;
//struct NetworkSenderEvent;

struct NetworkSenderEvent {
	Netcode::MessageType type;
	Netcode::MessageData* data = nullptr;  
};


class NWrapperSingleton : public NetworkEventHandler {
public:
	virtual ~NWrapperSingleton();

	// Initializes NetworkWrapper as NetworkWrapperHost
	bool host(int port = 54000);
	// Initializes NetworkWrapper as NetworkWrapperClient
	bool connectToIP(char* = "127.0.0.1:54000");
	bool isHost();
	void resetWrapper();
	void resetNetwork();
	NWrapper* getNetworkWrapper();
	void searchForLobbies();
	void checkFoundPackages();

	void resetPlayerList();
	bool playerJoined(Player& player);
	bool playerLeft(unsigned char& id);

	Player& getMyPlayer();
	Player* getPlayer(unsigned char& id);
	const std::list<Player>& getPlayers() const;
	void setPlayerName(const char* name);
	void setPlayerID(const unsigned char ID);
	std::string& getMyPlayerName();
	unsigned char getMyPlayerID();

	// Specifically for One-Time-Events during the gamestate
	void setNSS(NetworkSenderSystem* NSS);
	void queueGameStateNetworkSenderEvent(Netcode::MessageType type, Netcode::MessageData* messageData);
private:
	// Specifically for One-Time-Events during the gamestate
	NetworkSenderSystem* NSS = nullptr;
private:
	// Called by 'host' & 'connectToIP'
	void initialize(bool asHost);
	Network* m_network = nullptr;
	NWrapper* m_wrapper = nullptr;
	bool m_isInitialized = false;
	bool m_isHost = false;

	unsigned int m_playerCount;
	unsigned int m_playerLimit;

	Player m_me;
	std::list<Player> m_players;

	void handleNetworkEvents(NetworkEvent nEvent);

	// -+-+-+-+-+- Singleton requirements below -+-+-+-+-+-
public:
	NWrapperSingleton(NWrapperSingleton const&) = delete;
	void operator=(NWrapperSingleton const&) = delete;
	static NWrapperSingleton& getInstance();
private:
	NWrapperSingleton();
};