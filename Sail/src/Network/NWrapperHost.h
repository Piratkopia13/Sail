#pragma once

#include "NWrapper.h"
#include <string>

class NWrapperHost : public NWrapper {
public:
	NWrapperHost(Network* pNetwork) : NWrapper(pNetwork) {}
	virtual ~NWrapperHost() {}

	bool host(int port = 54000);
	bool connectToIP(char* = "127.0.0.1:54000");
	void setLobbyName(std::string name);
	void updateServerDescription();

#ifdef DEVELOPMENT
	const std::map<TCP_CONNECTION_ID, unsigned char>& getConnectionMap();
	const std::string& getServerDescription();
	const std::string& getLobbyName();
#endif // DEVELOPMENT


private:
	std::map<TCP_CONNECTION_ID, Netcode::PlayerID> m_connectionsMap;
	unsigned char m_IdDistribution = 0;
	std::string m_lobbyName = "";
	std::string m_serverDescription = "";

	void sendChatMsg(std::string msg);

	void playerJoined(TCP_CONNECTION_ID tcp_id);
	void playerDisconnected(TCP_CONNECTION_ID tcp_id);
	void playerReconnected(TCP_CONNECTION_ID tcp_id);

	void decodeMessage(NetworkEvent nEvent);
	void updateClientName(TCP_CONNECTION_ID tcp_id, Netcode::PlayerID playerId, std::string& name);

	/*
		This will request clients to enter a new state. GameState, EndGameState etc.
		id == 0 will send to all
	*/
	void setClientState(States::ID state, Netcode::PlayerID id = 0);
	virtual void kickPlayer(Netcode::PlayerID playerId);
	virtual void updateGameSettings(std::string s);

	virtual void requestTeam(char team);
	virtual void setTeamOfPlayer(char team, Netcode::PlayerID playerID, bool dispatch = true);
};