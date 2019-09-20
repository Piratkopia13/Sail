#include "pch.h"
#include "NWrapperSingleton.h"
#include "Network/NetworkModule.hpp"
#include "Network/NetworkStructs.hpp"
#include "Sail.h"

NWrapperSingleton::~NWrapperSingleton() {
	if (m_isInitialized && m_wrapper != nullptr) {
		delete m_wrapper;
		if (m_network != nullptr) {
			m_network->shutdown();
			delete m_network;
		}
	}
}

NWrapperSingleton::NWrapperSingleton() {
	m_network = new Network;
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
			m_wrapper = new NWrapperHost(m_network);
		}
		else {
			m_isHost = false;
			m_wrapper = new NWrapperClient(m_network);
		}
	}
}

void NWrapperSingleton::resetNetwork() {
	m_isInitialized = false;
	m_isHost = false;
	delete this->m_wrapper;
}