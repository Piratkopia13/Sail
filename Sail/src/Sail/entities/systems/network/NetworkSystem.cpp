#include "pch.h"
#include "NetworkSystem.h"
#include "Network/NWrapperSingleton.h"

NetworkSystem::NetworkSystem() {
	m_network = NWrapperSingleton::getInstance().getNetworkWrapper();
}

void NetworkSystem::initialize(Entity* playerEntity) {
	m_playerEntity = playerEntity;
}