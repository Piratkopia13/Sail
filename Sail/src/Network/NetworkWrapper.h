#pragma once

#include <string>
#include "Network/NetworkStructs.hpp"

// Forward declaration :)
class Network;

class NetworkWrapper : NetworkEventHandler
{
public:

	static NetworkWrapper& getInstance() {
		static NetworkWrapper instance; // Guaranteed to be destroyed.
										// Instantiated on first use.
		return instance;
	}

	NetworkWrapper(NetworkWrapper const&) = delete;
	void operator=(NetworkWrapper const&) = delete;

	void Initialize();
	
	~NetworkWrapper();

	bool host(int port = 54000);

	/*
		Connects to the IP and port in the format "192.168.1.55:54000".
	*/
	bool connectToIP(char* = "127.0.0.1:54000");
	void sendMsg(std::string msg);

	/*
		Send chat message to host.
	*/
	void sendChatMsg(std::string msg);
	void sendMsgAllClients(std::string msg);

	/*
		Call this in an update loop.
	*/
	void checkForPackages();

	bool isInitialized();
	bool isHost();

private:
	Network* m_network;


	NetworkWrapper() {}

	/*
		This is the general message decoder who does different things depending on starting letter.
	*/
	void decodeMessage(NetworkEvent nEvent);

	/*
		Only the host will get these messages to then send to each client.
	*/
	void playerDisconnected(ConnectionID id);
	void playerReconnected(ConnectionID id); // This remains unimplemented.
	void playerJoined(ConnectionID id);

	/*
		Depending on event, call the correct function.
	*/
	void handleNetworkEvents(NetworkEvent nEvents);

};

