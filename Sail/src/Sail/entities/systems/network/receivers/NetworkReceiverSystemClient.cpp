#include "pch.h"
#include "NetworkReceiverSystemClient.h"

#include "Network/NWrapperSingleton.h"
#include "../SPLASH/src/game/states/GameState.h"

NetworkReceiverSystemClient::NetworkReceiverSystemClient() {

}

NetworkReceiverSystemClient::~NetworkReceiverSystemClient() {

}

void NetworkReceiverSystemClient::mergeHostsStats() {
}
void NetworkReceiverSystemClient::handleIncomingData(std::string data) {
	pushDataToBuffer(data);
}

void NetworkReceiverSystemClient::endMatch() {

	m_gameStatePtr->requestStackPop();
	m_gameStatePtr->requestStackPush(States::EndGame);
}

void NetworkReceiverSystemClient::endMatchAfterTimer() {
}

void NetworkReceiverSystemClient::prepareEndScreen(int bf, float dw, int jm, Netcode::PlayerID id) {
}
