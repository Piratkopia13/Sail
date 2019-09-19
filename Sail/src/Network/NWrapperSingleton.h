#pragma once


#include "NetworkWrapper.h"
#include "NWrapperHost.h"
#include "NWrapperClient.h"

class NWrapperSingleton {
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
	NWrapper* getNetworkWrapper();

private:
	NWrapperSingleton() { }
	// Called by 'host' & 'connectToIP'
	void initialize(bool asHost);

	NWrapper* m_wrapper = nullptr;
	bool m_isInitialized = false;
	bool m_isHost = false;
};