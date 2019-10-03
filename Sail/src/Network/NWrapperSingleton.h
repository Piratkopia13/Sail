#pragma once

#include "NWrapperHost.h"
#include "NWrapperClient.h"

class NWrapperSingleton : public NetworkEventHandler {
public:
	// Guaranteed to be destroyed, instantiated on first use.
	static NWrapperSingleton& getInstance() {
		static NWrapperSingleton instance;
		return instance;
	}

	~NWrapperSingleton();
	NWrapperSingleton(NWrapperSingleton const&) = delete;
	void operator=(NWrapperSingleton const&) = delete;

	// Initializes NetworkWrapper as NetworkWrapperHost
	bool host(int port = 54000);
	// Initializes NetworkWrapper as NetworkWrapperClient
	bool connectToIP(char* = "127.0.0.1:54000");
	bool isHost();
	void resetNetwork();
	NWrapper* getNetworkWrapper();
	void searchForLobbies();
	void checkFoundPackages();

private:
	NWrapperSingleton();
	// Called by 'host' & 'connectToIP'
	void initialize(bool asHost);
	Network* m_network = nullptr;
	NWrapper* m_wrapper = nullptr;
	bool m_isInitialized = false;
	bool m_isHost = false;

	void handleNetworkEvents(NetworkEvent nEvent);
};