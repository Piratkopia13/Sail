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

void NetworkReceiverSystemHost::endMatchAfterTimer(float dt) {

	static float endGameClock;
	if (m_startEndGameTimer) {
		endGameClock += dt;
	}
	if (endGameClock > 2.0f) {
		NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
			Netcode::MessageType::ENDGAME_STATS,
			nullptr,
			false
		);

		m_gameStatePtr->requestStackClear();
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
		gts->bulletsFiredID = id;
	}
	if (dw > gts->distanceWalked) {
		gts->distanceWalked = dw;
		gts->distanceWalkedID = id;
	}
	if (jm > gts->jumpsMade) {
		gts->jumpsMade = jm;
		gts->jumpsMadeID = id;
	}

	// Send data back in Netcode::MessageType::ENDGAME_STATS
	endMatch(); // Starts the end game timer. Runs only for the host

}

void NetworkReceiverSystemHost::mergeHostsStats() {

	GameDataTracker* gdt = &GameDataTracker::getInstance();
	Netcode::PlayerID id = NWrapperSingleton::getInstance().getMyPlayerID();

	if (gdt->getStatisticsLocal().bulletsFired > gdt->getStatisticsGlobal().bulletsFired) {
		gdt->getStatisticsGlobal().bulletsFired = gdt->getStatisticsLocal().bulletsFired;
		gdt->getStatisticsGlobal().bulletsFiredID = id;
	}
	if (gdt->getStatisticsLocal().distanceWalked > gdt->getStatisticsGlobal().distanceWalked) {
		gdt->getStatisticsGlobal().distanceWalked = gdt->getStatisticsLocal().distanceWalked;
		gdt->getStatisticsGlobal().distanceWalkedID = id;
	}
	if (gdt->getStatisticsLocal().jumpsMade > gdt->getStatisticsGlobal().jumpsMade) {
		gdt->getStatisticsGlobal().jumpsMade = gdt->getStatisticsLocal().jumpsMade;
		gdt->getStatisticsGlobal().jumpsMadeID = id;
	}

	endMatch();
}

void NetworkReceiverSystemHost::waterHitPlayer(Netcode::ComponentID id, Netcode::PlayerID senderId) {
	EventDispatcher::Instance().emit(WaterHitPlayerEvent(id, senderId));

	//for (auto& e : entities) {
	//	//Look for the entity that OWNS the candle (player entity)
	//	if (e->getComponent<NetworkReceiverComponent>()->m_id != id) {
	//		continue;
	//	}
	//	//Look for the entity that IS the candle (candle entity)
	//	std::vector<Entity*> childEntities = e->getChildEntities();
	//	for (auto& child : childEntities) {
	//		if (child->hasComponent<CandleComponent>()) {
	//			// Damage the candle
	//			// Save the Shooter of the Candle if its lethal
	//			// TODO: Replace 10.0f with game settings damage
	//			child->getComponent<CandleComponent>()->hitWithWater(10.0f, senderId);
	//
	//			// Play relevant sound
	//			if (child->getComponent<CandleComponent>()->isLit) {
	//				if (e->hasComponent<LocalOwnerComponent>()) {
	//					e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::WATER_IMPACT_MY_CANDLE].isPlaying = true;
	//					e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::WATER_IMPACT_MY_CANDLE].playOnce = true;
	//				} else {
	//					e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::WATER_IMPACT_ENEMY_CANDLE].isPlaying = true;
	//					e->getComponent<AudioComponent>()->m_sounds[Audio::SoundType::WATER_IMPACT_ENEMY_CANDLE].playOnce = true;
	//				}
	//			}
	//
	//			// Check in Candle System What happens next
	//			return;
	//		}
	//	}
	//}
	//SAIL_LOG_WARNING("waterHitPlayer called but no matching entity found");
}