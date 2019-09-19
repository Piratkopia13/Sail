#include "pch.h"
#include "NWrapperSingleton.h"
#include "Sail.h"

NWrapperSingleton::~NWrapperSingleton() {
	if (m_isInitialized && m_wrapper != nullptr) {
		delete m_wrapper;
	}
}

bool NWrapperSingleton::host(int port) {
	this->initialize(true);

	if (m_isHost) {
		return m_wrapper->host(port);
	}

	return false;
}

bool NWrapperSingleton::connectToIP(char* adress) {
	this->initialize(false);
	
	if (!m_isHost) {
		return m_wrapper->connectToIP(adress);
	}
	
	return false;
}

bool NWrapperSingleton::isHost() {
	return m_isHost;
}

NWrapper* NWrapperSingleton::getNetworkWrapper() {
	return m_wrapper;
}

void NWrapperSingleton::initialize(bool asHost) {
	if (m_isInitialized == false) {
		m_isInitialized = true;

		if (asHost) {
			m_isHost = true;
			m_wrapper = new NWrapperHost();
		}
		else {
			m_isHost = false;
			m_wrapper = new NWrapperClient();
		}
	}
}
