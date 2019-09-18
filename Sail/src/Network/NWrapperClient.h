#pragma once

#include "NWrapper.h"

class NWrapperClient : public NWrapper {
public:
	NWrapperClient() {}
	~NWrapperClient() {}

private:

	void playerJoined(TCP_CONNECTION_ID id);
	void playerDisconnected(TCP_CONNECTION_ID id);
	void playerReconnected(TCP_CONNECTION_ID id);
	void decodeMessage(NetworkEvent nEvent);
};