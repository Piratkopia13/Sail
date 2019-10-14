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

	m_playerLimit = 12;
	m_playerCount = 0;
}

bool NWrapperSingleton::host(int port) {
	this->initialize(true);

	if (m_isHost) {
		if (m_wrapper->host(port) == true) {
			return true;
		} else {
			resetWrapper();
		}
	}

	return false;
}

bool NWrapperSingleton::connectToIP(char* adress) {
	this->initialize(false);
	
	if (!m_isHost) {
		if (m_wrapper->connectToIP(adress) == true) {
			return true;
		} else {
			resetWrapper();
		}
	}
	
	return false;
}

bool NWrapperSingleton::isHost() {
	return m_isHost;
}

NWrapper* NWrapperSingleton::getNetworkWrapper() {
	return m_wrapper;
}

void NWrapperSingleton::searchForLobbies() {
	m_network->searchHostsOnLan();
}

void NWrapperSingleton::checkFoundPackages() {
	m_network->checkForPackages(*this);
}

void NWrapperSingleton::resetPlayerList()
{
	m_players.clear();
	m_playerCount = 0;
}

bool NWrapperSingleton::playerJoined(Player& player) {
	if (m_playerCount < m_playerLimit) {
		m_players.push_back(player);
		m_playerCount++;
		return true;
	}
	return false;
}

bool NWrapperSingleton::playerLeft(unsigned char& id) {
	// Linear search to get target 'player' struct, then erase that from the list
	Player* toBeRemoved = nullptr;
	int pos = 0;
	for (auto playerIt : m_players) {
		if (playerIt.id == id) {
			toBeRemoved = &playerIt;
			m_players.remove(*toBeRemoved);
			return true;
		}
	}

	return false;
}

Player& NWrapperSingleton::getMyPlayer() {
	return m_me;
}

Player* NWrapperSingleton::getPlayer(unsigned char& id) {
	Player* foundPlayer = nullptr;
	for (Player& player : m_players) {
		if (player.id == id) {
			foundPlayer = &player;
			break;
			//return foundPlayer;
		}
	}

	return foundPlayer;
}

const std::list<Player>& NWrapperSingleton::getPlayers() const {
	return m_players;
}


void NWrapperSingleton::setPlayerName(const char* name) {
	m_me.name = name;
}

void NWrapperSingleton::setPlayerID(const unsigned char ID) {
	m_me.id = ID;
}

std::string& NWrapperSingleton::getMyPlayerName() {
	return m_me.name;
}

unsigned char NWrapperSingleton::getMyPlayerID() {
	return m_me.id;
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

void NWrapperSingleton::resetWrapper() {
	m_isInitialized = false;
	m_isHost = false;
	delete this->m_wrapper;
}

void NWrapperSingleton::resetNetwork() {
	m_network->shutdown();
	m_network->initialize();
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
