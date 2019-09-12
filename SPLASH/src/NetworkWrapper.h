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
	void SendChat(std::string msg);
	void CheckForPackages(NetworkEvent nEvent);

private:
	Network* m_network;
	

};
