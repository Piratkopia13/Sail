#pragma once

#include <string>
#include "Network/NetworkStructs.hpp"

// Forward declaration
class Network;

class NetworkWrapper
{
public:
	NetworkWrapper();
	~NetworkWrapper();

	bool Host(int port = 54000);
	bool ConnectToIP(const char* ip = "127.0.0.1", int port = 54000);
	void SendChat(char* msg);
	void SendChatAllClients(char* msg);
	void ProcessPackages(NetworkEvent nEvent);
	void CheckForPackages();

private:
	Network* m_network;

	void DecodeMessage(NetworkEvent nEvent);
	void PlayerDisconnected(ConnectionID id);
	void PlayerReconnected(ConnectionID id); // This remains unimplemented.
	void PlayerJoined(ConnectionID id);

};

