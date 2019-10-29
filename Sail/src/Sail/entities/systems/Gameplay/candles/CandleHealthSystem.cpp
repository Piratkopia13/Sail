#include "pch.h"
#include "CandleHealthSystem.h"
#include "Sail/entities/components/Components.h"
#include "Network/NWrapperSingleton.h"
#include "Sail/utils/GameDataTracker.h"

CandleHealthSystem::CandleHealthSystem() {
	registerComponent<CandleComponent>(true, true, true);
	registerComponent<NetworkReceiverComponent>(false, true, false);
	registerComponent<LightComponent>(true, true, true);
}

CandleHealthSystem::~CandleHealthSystem() {}

void CandleHealthSystem::update(float dt) {
	// The number of living candles, representing living players
	int livingCandles = entities.size();

	for (auto e : entities) {
		auto candle = e->getComponent<CandleComponent>();

		if (candle->isLit) {
			// Decrease invincibility time
			candle->invincibleTimer -= dt;
			candle->invincibleTimer = candle->invincibleTimer < 0.f ? 0.f : candle->invincibleTimer;

			// If candle is alive
			if (candle->health > 0.f) {
				if (candle->damageTakenLastHit != 0.f && candle->invincibleTimer <= 0.f) {
					candle->health -= candle->damageTakenLastHit;
				} else {
					continue;
				}
			} else {
				if (candle->respawns == m_maxNumRespawns) {
					livingCandles--;

					//Only let the host send PLAYER_DIED message
					if (NWrapperSingleton::getInstance().isHost()) {
						NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
							Netcode::MessageType::PLAYER_DIED,
							SAIL_NEW Netcode::MessagePlayerDied{
								e->getParent()->getComponent<NetworkReceiverComponent>()->m_id,
								candle->wasHitByPlayerID
							}
						);

						// Save the placement for the player who lost
						GameDataTracker::getInstance().logPlacement(
							Netcode::getComponentOwner(e->getParent()->getComponent<NetworkReceiverComponent>()->m_id)
						);
					}
				} else {
					candle->health = 0.f;
					candle->invincibleTimer = 0.f;
					candle->respawns++;
				}
			}

			// Reset candle's damage taken
			candle->damageTakenLastHit = 0.f;

			// COLOR/INTENSITY
			float cHealth = std::fmaxf(candle->health, 0.f);
			float tempHealthRatio = (cHealth / MAX_HEALTH);
			e->getComponent<LightComponent>()->getPointLight().setColor(glm::vec3(tempHealthRatio, tempHealthRatio * 0.7f, tempHealthRatio * 0.4f));
		}
	}

	if (livingCandles < 2) { // Match IS over
		NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
			Netcode::MessageType::MATCH_ENDED,
			nullptr
		);
		// Send relevant stats to all clients
		NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
			Netcode::MessageType::ENDGAME_STATS,
			nullptr,
			false
		);
	}
}
