#pragma once

#include <string>
#include "Network/NetworkStructs.hpp"

// Forward declaration
class Network;

//void ProcessPackage(NetworkEvent nEvent) {
//
//	// 1m -> message
//	// 2n -> Send player name to host
//	// 3p -> pos?
//	//int c = p.msg[0];
//
//
//	// Remove the first letter in the message.
//	//char data[sizeof(p.msg)-1];
//
//	/*if(strlen(p.msg) > 0)
//	{
//	    strcpy(data, &(p.msg[1]));
//	}
//	else
//	{
//	    strcpy(data, p.msg);
//	}*/
//
//	//switch (c)
//	//{
//	//case 1:
//	//	// Print message to GUI Chat with p.senderId, '1: Hejsan'
//	//	printf(data);
//	//	break;
//	//case 2:
//	//	// Map the p.senderId with data
//	//	break;
//
//	//default:
//	//	// Do nothing?
//	//	break;
//	//}
//
//
//
//}

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

