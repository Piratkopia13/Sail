#pragma once

#include <string>
#include "Network/NetworkStructs.hpp"

// Forward declaration
class Network;

class NetworkWrapper : NetworkEventHandler
{
public:
	NetworkWrapper();
	~NetworkWrapper();

	bool Host(int port = 54000);
	bool ConnectToIP(const char* ip = "127.0.0.1", int port = 54000);
	void SendChat(std::string msg);
	void SendChatAllClients(std::string msg);
	void CheckForPackages();

	bool isInitialized();

private:
	Network* m_network;

	void DecodeMessage(NetworkEvent nEvent);
	void PlayerDisconnected(ConnectionID id);
	void PlayerReconnected(ConnectionID id); // This remains unimplemented.
	void PlayerJoined(ConnectionID id);
	void handleNetworkEvents(NetworkEvent nEvents);

};

