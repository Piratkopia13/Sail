#include "pch.h"
#include "NetworkReceiverSystemHost.h"
#include "../NetworkSenderSystem.h"
#include "Network/NWrapperSingleton.h"
#include "Sail/utils/Utils.h"
#include "Sail/utils/GameDataTracker.h"

#include "../SPLASH/src/game/states/GameState.h"

NetworkReceiverSystemHost::NetworkReceiverSystemHost() {

}

NetworkReceiverSystemHost::~NetworkReceiverSystemHost() {

}


void NetworkReceiverSystemHost::handleIncomingData(std::string data) {
	pushDataToBuffer(data);

	// The host will also save the data in the sender system so that it can be forwarded to all other clients
	m_netSendSysPtr->pushDataToBuffer(data);
}

void NetworkReceiverSystemHost::endMatch() {

	m_startEndGameTimer = true;
}

void NetworkReceiverSystemHost::endMatchAfterTimer() {

	static float endGameClock;
	if (m_startEndGameTimer) {
		endGameClock++;
	}
	if (endGameClock > 100.0f) {
		NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
			Netcode::MessageType::ENDGAME_STATS,
			nullptr,
			false
		);

		m_gameStatePtr->requestStackPop();
		m_gameStatePtr->requestStackPush(States::EndGame);
	}
}

void NetworkReceiverSystemHost::prepareEndScreen(int bf, float dw, int jm, Netcode::PlayerID id) {
	GameDataTracker* dgtp = &GameDataTracker::getInstance();

	// Process the data
	if (bf > dgtp->getStatistics().bulletsFired) {
		dgtp->getStatistics().bulletsFired = bf;
		dgtp->getStatistics().bfID = id;
	}
	if (dw > dgtp->getStatistics().distanceWalked) {
		dgtp->getStatistics().distanceWalked = dw;
		dgtp->getStatistics().dwID = id;
	}
	if (jm > dgtp->getStatistics().jumpsMade) {
		dgtp->getStatistics().jumpsMade = jm;
		dgtp->getStatistics().jmID = id;
	}

	// Send data back in Netcode::MessageType::ENDGAME_STATS
	endMatch(); // Starts the end game timer. Runs only for the host

}
