#pragma once



#include "Network/NetworkStructs.hpp"

class Network;

class NWrapper : public NetworkEventHandler {
public:
	NWrapper() {}
	~NWrapper() {}

	NWrapper(NWrapper const&) = delete;
	void operator=(NWrapper const&) = delete;



	static void initAsHost();
	static void initAsClient();

	static NWrapper* getInstance();


	void handleNetworkEvents(NetworkEvent nEvent);

protected:
	Network* m_network;

private:
	NWrapper() {}
	static bool isHost;
	static bool isInitialized;

	virtual void playerJoined(TCP_CONNECTION_ID id) = 0;
	virtual void playerDisconnected(TCP_CONNECTION_ID id) = 0;
	virtual void playerReconnected(TCP_CONNECTION_ID id) = 0;
	virtual void decodeMessage(NetworkEvent nEvent) = 0;
};