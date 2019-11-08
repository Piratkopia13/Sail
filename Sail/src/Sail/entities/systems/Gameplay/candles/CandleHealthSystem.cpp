#include "pch.h"
#include "CandleHealthSystem.h"
#include "Sail/entities/components/Components.h"
#include "Network/NWrapperSingleton.h"
#include "Sail/utils/GameDataTracker.h"

CandleHealthSystem::CandleHealthSystem() {
	registerComponent<CandleComponent>(true, true, true);
	registerComponent<NetworkSenderComponent>(false, true, false);
	registerComponent<LightComponent>(true, true, true);
	registerComponent<AudioComponent>(false, true, true);
}

CandleHealthSystem::~CandleHealthSystem() {}

void CandleHealthSystem::update(float dt) {
	// The number of living candles, representing living players
	int livingCandles = entities.size();
	for (auto e : entities) {
		auto candle = e->getComponent<CandleComponent>();

		candle->wasHitByMeThisTick = false;

		if (candle->isLit) {
			// Decrease invincibility time
			candle->invincibleTimer -= dt;

			// If candle is alive
			if (candle->health > 0.f) {
				if (candle->damageTakenLastHit != 0.f && candle->invincibleTimer <= 0.f) {
					candle->health -= candle->damageTakenLastHit;
					// TODO: Replace 0.4f with game settings
					candle->invincibleTimer = 0.4f;
				}
			// Candle has been extinguished
			} else {
				candle->health = 0.f;
				candle->invincibleTimer = 0.f;
				candle->isLit = false;

				if (candle->lastDamageSource == CandleComponent::DamageSource::PLAYER) {
					GameDataTracker::getInstance().logEnemyKilled(candle->wasHitByPlayerID);
				}

				// No respawns left - die!
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
					auto playerEntity = e->getParent();
					playerEntity->getComponent<AudioComponent>()->m_sounds[Audio::RE_IGNITE_CANDLE].isPlaying = true;
				}
			}
		}

		// COLOR/INTENSITY
		float cHealth = std::fmaxf(candle->health, 0.f);
		float tempHealthRatio = (cHealth / MAX_HEALTH);
		e->getComponent<LightComponent>()->getPointLight().setColor(glm::vec3(tempHealthRatio, tempHealthRatio * 0.7f, tempHealthRatio * 0.4f));

		// Reset candle's damage taken
		candle->damageTakenLastHit = 0.f;
	}

	// Only one living candle left and number of players in the game is greater than one
	if (NWrapperSingleton::getInstance().isHost()) {
		if (livingCandles < 2 && entities.size() > 1) {
			NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
				Netcode::MessageType::MATCH_ENDED,
				nullptr
			);
		}
	}
}
