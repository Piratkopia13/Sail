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
	const bool isHost = NWrapperSingleton::getInstance().isHost();

	// The number of living candles, representing living players
	size_t livingCandles = entities.size();

	for (auto e : entities) {
		auto candle = e->getComponent<CandleComponent>();
		candle->wasHitByMeThisTick = false;


#pragma region HOST_ONLY_STUFF
		if (isHost && candle->isLit) {
			if (candle->health > 0.0f) {
				candle->invincibleTimer -= dt;

				// If someone hit the candle this tick tell all players to update the candle's health
				if (candle->wasHitThisTick) {
					// Candle has lost all its health so extinguish it
					NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
						Netcode::MessageType::SET_CANDLE_HEALTH,
						SAIL_NEW Netcode::MessageSetCandleHealth{
							e->getComponent<NetworkReceiverComponent>()->m_id,
							candle->health
						},
						false // Host already knows the candle's health so don't send to ourselves
					);
					candle->wasHitThisTick = false;
				}
			} else { // If candle used to be lit but has lost all its health
				candle->wasJustExtinguished = true;

				// Candle has lost all its health so extinguish it
				NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
					Netcode::MessageType::EXTINGUISH_CANDLE,
					SAIL_NEW Netcode::MessageExtinguishCandle{
						e->getComponent<NetworkReceiverComponent>()->m_id,
						candle->wasHitByPlayerID
					},
					false // Host extinguishes candle later in this function so don't send to ourself
				);

				// If the player has no more respawns kill them
				if (candle->respawns >= m_maxNumRespawns) {
					livingCandles--;

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

					// Only one living candle left and number of players in the game is greater than one
					if (livingCandles < 2 && entities.size() > 1) {
						NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
							Netcode::MessageType::MATCH_ENDED,
							nullptr
						);
					}
				}
			}
		}
#pragma endregion


		// Everyone does this
		if (candle->wasJustExtinguished) {
			candle->health = 0.0f;
			candle->isLit = false;
			candle->wasJustExtinguished = false; // reset for the next tick
			if (candle->wasHitByPlayerID != Netcode::MESSAGE_SPRINKLER_ID) {
				GameDataTracker::getInstance().logEnemyKilled(candle->wasHitByPlayerID);
			}
		
			// Play the reignition sound if the player has any candles left
			if (candle->respawns < m_maxNumRespawns) {
				auto playerEntity = e->getParent();
				playerEntity->getComponent<AudioComponent>()->m_sounds[Audio::RE_IGNITE_CANDLE].isPlaying = true;
			}
		}

		// COLOR/INTENSITY
		float cHealth = std::fmaxf(candle->health, 0.f);
		float tempHealthRatio = (cHealth / MAX_HEALTH);
		e->getComponent<LightComponent>()->getPointLight().setColor(glm::vec3(tempHealthRatio, tempHealthRatio * 0.7f, tempHealthRatio * 0.4f));
	}
}
