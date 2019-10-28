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

		// Reset clock for next session
		m_startEndGameTimer = false;
		endGameClock = 0;
	}
}

void NetworkReceiverSystemHost::prepareEndScreen(int bf, float dw, int jm, Netcode::PlayerID id) {

	GlobalTopStats* gts = &GameDataTracker::getInstance().getStatisticsGlobal();

	// Process the data
	if (bf > gts->bulletsFired) {
		gts->bulletsFired = bf;
		gts->bfID = id;
	}
	if (dw > gts->distanceWalked) {
		gts->distanceWalked = dw;
		gts->dwID = id;
	}
	if (jm > gts->jumpsMade) {
		gts->jumpsMade = jm;
		gts->jmID = id;
	}

	// Send data back in Netcode::MessageType::ENDGAME_STATS
	endMatch(); // Starts the end game timer. Runs only for the host

}
