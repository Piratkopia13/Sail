#include "pch.h"
#include "NWrapperSingleton.h"
#include "Network/NetworkModule.hpp"
#include "Network/NetworkStructs.hpp"
#include "Sail.h"
#include "../../SPLASH/src/game/events/NetworkLanHostFoundEvent.h"


NWrapperSingleton::~NWrapperSingleton() {
	if (m_isInitialized && m_wrapper != nullptr) {
		delete m_wrapper;
		
	}

	Memory::SafeDelete(m_network);

}

NWrapperSingleton::NWrapperSingleton() {
	m_network = SAIL_NEW Network;
	m_network->initialize();
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

void NWrapperSingleton::checkForLobbies() {
	m_network->searchHostsOnLan();
	m_network->checkForPackages(*this);
}

void NWrapperSingleton::initialize(bool asHost) {
	if (m_isInitialized == false) {
		m_isInitialized = true;

		if (asHost) {
			m_isHost = true;
			m_wrapper = SAIL_NEW NWrapperHost(m_network);
		}
		else {
			m_isHost = false;
			m_wrapper = SAIL_NEW NWrapperClient(m_network);
		}
	}
}

void NWrapperSingleton::resetNetwork() {
	m_isInitialized = false;
	m_isHost = false;
	delete this->m_wrapper;
}

void NWrapperSingleton::handleNetworkEvents(NetworkEvent nEvent) {
	if (nEvent.eventType == NETWORK_EVENT_TYPE::HOST_ON_LAN_FOUND) {
		NetworkLanHostFoundEvent event(
			nEvent.data->HostFoundOnLanData.ip_full,
			nEvent.data->HostFoundOnLanData.hostPort
		);
		Application::getInstance()->dispatchEvent(event);
	}
}
