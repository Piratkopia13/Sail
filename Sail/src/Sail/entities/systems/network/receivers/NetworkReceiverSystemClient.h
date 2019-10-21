#pragma once

#include "NetworkReceiverSystem.h"



class NetworkReceiverSystemClient : public NetworkReceiverSystem {
public:
	NetworkReceiverSystemClient();
	~NetworkReceiverSystemClient();

	// Push incoming data strings to the back of a FIFO list
	void pushDataToBuffer(std::string data) override;

private:

};