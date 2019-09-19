#pragma once

#include "Network/NetworkStructs.hpp"
#include "../../SPLASH/src/game/states/LobbyState.h"	// Needed for 'Message'
#include "Sail.h"
#include <string>

class Network;

class NWrapper : public NetworkEventHandler {
public:
	NWrapper();
	virtual ~NWrapper();

	virtual bool host(int port = 54000) = 0;
	virtual bool connectToIP(char* = "127.0.0.1:54000") = 0;
	void checkForPackages();

	void sendMsg(std::string msg);					// Some of these probably only get used
	void sendMsgAllClients(std::string msg);		// by either client or host
	void sendChatAllClients(std::string msg);		//
	virtual void sendChatMsg(std::string msg) = 0;

protected:
	Network* m_network = nullptr;
	Application* m_app = nullptr;	

	// Parsing functions | Will alter 'data' upon being used.
	TCP_CONNECTION_ID parseID(std::string& data);	//
	std::string parseName(std::string& data);		//
	Message processChatMessage(std::string& message);

	// Formatting Functions
	void compressDCMessage();
	void decompressDCmessage();

private:
	void initialize();
	void shutDown();
	void handleNetworkEvents(NetworkEvent nEvent);

	virtual void playerJoined(TCP_CONNECTION_ID id) = 0;
	virtual void playerDisconnected(TCP_CONNECTION_ID id) = 0;
	virtual void playerReconnected(TCP_CONNECTION_ID id) = 0;
	virtual void decodeMessage(NetworkEvent nEvent) = 0;
};