#pragma once

#include "NWrapper.h"
#include <string>

#include "Sail/netcode/NetcodeTypes.h"

class NWrapperHost : public NWrapper {
public:
	NWrapperHost(Network* pNetwork) : NWrapper(pNetwork) {}
	virtual ~NWrapperHost() {}

	bool host(int port = 54000);
	bool connectToIP(char* = "127.0.0.1:54000");
	void setLobbyName(std::string name);
	void updateServerDescription();

	void sendSerializedDataToClient(std::string data, Netcode::PlayerID playerID) override;
private:
	std::map<TCP_CONNECTION_ID, Netcode::PlayerID> m_connectionsMap;
	Netcode::PlayerID m_IdDistribution = 0;
	std::string m_lobbyName = "";
	std::string m_serverDescription = "";

	void sendChatMsg(std::string msg);

	void playerJoined(TCP_CONNECTION_ID id);
	void playerDisconnected(TCP_CONNECTION_ID id);
	void playerReconnected(TCP_CONNECTION_ID id);
	void decodeMessage(NetworkEvent nEvent);

	// Formatting Functions
	void compressDCMessage(unsigned char& convertedId, char pDestination[64]);
};