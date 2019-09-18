#pragma once

#include <string>
#include "Network/NetworkStructs.hpp"
#include <map>
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

	void initialize();
	
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
	void sendChatAllClients(std::string msg);

	/*
		Call this in an update loop.
	*/
	void checkForPackages();

	bool isInitialized();
	bool isHost();

private:
	Network* m_network;
	NetworkWrapper() {}

	// Map of all connection IDs and the assigned ID of each connection of the session.
	std::map<TCP_CONNECTION_ID, unsigned char> m_connectionsMap;
	unsigned char m_IdDistribution = 0;

	// TODO: Shutdown function which will be used to re-host or re-join lobbies.

	/*
		This is the general message decoder who does different things depending on starting letter.
	*/
	void decodeMessage(NetworkEvent nEvent);

	/*
		Only the host will get these messages to then send to each client.
	*/
	void playerDisconnected(TCP_CONNECTION_ID id);
	void playerReconnected(TCP_CONNECTION_ID id); // This remains unimplemented.
	void playerJoined(TCP_CONNECTION_ID id);

	/*
		Depending on event, call the correct function.
	*/
	void handleNetworkEvents(NetworkEvent nEvents);
	/*
		 WARNING: These parse functions EXTRACT data, NOT COPY.
		 They WILL remove the part they extract from data.
	*/
	
	TCP_CONNECTION_ID parseID(std::string& data);
	std::string parseName(std::string& data);


};

