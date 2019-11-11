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
	std::map<TCP_CONNECTION_ID, unsigned char> m_connectionsMap;
	unsigned char m_IdDistribution = 0;
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