#pragma once

#include "NWrapper.h"

class NWrapperClient : public NWrapper {
public:
	NWrapperClient(Network* pNetwork) : NWrapper(pNetwork) {}
	virtual ~NWrapperClient() {}

	bool host(int port = 54000);
	bool connectToIP(char* = "127.0.0.1:54000");

private:
	void sendChatMsg(std::string msg);

	void playerJoined(TCP_CONNECTION_ID id);
	void playerDisconnected(TCP_CONNECTION_ID id);
	void playerReconnected(TCP_CONNECTION_ID id);
	void decodeMessage(NetworkEvent nEvent);

	// Deformatting Functions
	unsigned int decompressDCMessage(std::string messageData);
};