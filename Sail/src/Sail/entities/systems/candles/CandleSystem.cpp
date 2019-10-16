#include "pch.h"
#include "CandleSystem.h"

#include "Sail/entities/components/LightComponent.h"
#include "Sail/entities/components/CandleComponent.h"
#include "Sail/entities/components/NetworkSenderComponent.h"
#include "Sail/entities/components/OnlineOwnerComponent.h"
#include "Sail/entities/components/TransformComponent.h"
#include "Sail/entities/components/GunComponent.h"
#include "Sail/entities/components/MovementComponent.h"
#include "Sail/entities/components/SpectatorComponent.h"
#include "Sail/entities/components/LocalOwnerComponent.h"
#include "Sail/entities/components/OnlineOwnerComponent.h"
#include "../Sail/src/Network/NWrapperSingleton.h"

#include "Sail/entities/Entity.h"

#include "Sail/graphics/camera/CameraController.h"

#include "Sail/entities/ECS.h"
#include "Sail/entities/systems/physics/UpdateBoundingBoxSystem.h"
#include "../Sail/src/Network/NWrapperSingleton.h"

#include "Sail/Application.h"

CandleSystem::CandleSystem() : BaseComponentSystem() {
	// TODO: System owner should check if this is correct
	registerComponent<CandleComponent>(true, true, true);
	registerComponent<TransformComponent>(true, true, false);
	registerComponent<LightComponent>(true, true, true);
}

CandleSystem::~CandleSystem() {

}

void CandleSystem::setPlayerEntityID(int entityID, Entity* entityPtr) {
	m_playerEntityID = entityID;
	m_playerEntityPtr = entityPtr;
	
}

// turn on the light of a specified candle if it doesn't have one already
void CandleSystem::lightCandle(const std::string& name) {
	for ( auto e : entities ) {
		if ( e->getName() == name ) {
			e->getComponent<LightComponent>()->getPointLight().setColor(glm::vec3(1.0f, 1.0f, 1.0f));
			break;
		}
	}
}

// should be updated after collision detection has been done
void CandleSystem::update(float dt) {
	int LivingCandles = entities.size();

	for (auto e : entities) {
		auto candle = e->getComponent<CandleComponent>();

		if ( candle->getIsAlive() ) {
			// Remove light from candles that were hit by projectiles
			if ( candle->wasHitByWater() ) {
				candle->resetHitByWater();

				if (candle->getInvincibleTimer() <= 0.f) {
					candle->decrementHealth(candle->getDamageTakenLastHit());
					candle->setInvincibleTimer(INVINCIBLE_DURATION);



					// A candle which is owned by a player has been hit
					// -- Does the candle belong to an online player?
					// -- Was it hit by the local player?
					
					// If the player is controlled through the network
					if (e->hasComponent<OnlineOwnerComponent>()) {
						CandleComponent* c = e->getComponent<CandleComponent>();

						// If the player who hit him was the local player
						if (c->hitByLocalPlayer == true) {
							// It (An online player) was hit by the local player
							NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
								Netcode::MessageType::WATER_HIT_PLAYER, 
								e // OLD
								/* SAIL_NEW Netcode::MessageDataWaterHitPlayer{ 
									e->getComponent<OnlineOwnerComponent>()->netEntityID
								}*/
							);
						}
					}
					

					if ( candle->getHealth() <= 0.f ) {
						candle->setIsLit(false);

						if (candle->getOwner() == m_playerEntityID) {
							if (!candle->isCarried()) {
								candle->toggleCarried();
							}
						}

						// Did current player die?
						if (candle->getNumRespawns() == m_maxNumRespawns) {
							candle->setIsAlive(false);

							// Check if the extinguished candle is owned by the player
							if (candle->getOwner() == m_playerEntityID) {
								LivingCandles--;

								if (LivingCandles <= 1) { // Match IS over
									NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
										Netcode::MessageType::MATCH_ENDED,
										nullptr
									);

									m_gameStatePtr->requestStackPop();
									m_gameStatePtr->requestStackPush(States::EndGame);
								}
								else { // Match IS NOT over, instead THIS player simply died
									e->getParent()->addComponent<SpectatorComponent>();
									e->getParent()->getComponent<MovementComponent>()->constantAcceleration = glm::vec3(0.f, 0.f, 0.f);
									e->getParent()->removeComponent<GunComponent>();
									e->getParent()->removeAllChildren();
									// TODO: Remove all the components that can/should be removed

									NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
										Netcode::MessageType::PLAYER_DIED,
										m_playerEntityPtr
									);
								}
							}
						}
					}
				}

			} else if ( (candle->getDoActivate() || candle->getDownTime() >= m_candleForceRespawnTimer) && !candle->getIsLit() ) {
				candle->setIsLit(true);
				candle->setHealth(MAX_HEALTH);
				candle->incrementRespawns();
				candle->resetDownTime();
				candle->resetDoActivate();
			} else if ( !candle->getIsLit() ) {
				candle->addToDownTime(dt);
			}

			if ( candle->isCarried() != candle->getWasCarriedLastUpdate() ) {
				putDownCandle(e);
			}

			if ( candle->getInvincibleTimer() > 0.f ) {
				candle->decrementInvincibleTimer(dt);
			}

			// COLOR/INTENSITY
			float cHealth = candle->getHealth();
			cHealth = (cHealth < 0.f) ? 0.f : cHealth;
			float tempHealthRatio = ( cHealth / MAX_HEALTH );
			e->getComponent<LightComponent>()->getPointLight().setColor(glm::vec3(tempHealthRatio, tempHealthRatio, tempHealthRatio));

			candle->setWasCarriedLastUpdate(candle->isCarried());
			glm::vec3 flamePos = glm::vec3(e->getComponent<TransformComponent>()->getMatrix()[3]) + glm::vec3(0, 0.5f, 0);
			e->getComponent<LightComponent>()->getPointLight().setPosition(flamePos);
		}
	}
}

void CandleSystem::putDownCandle(Entity* e) {
	auto candleComp = e->getComponent<CandleComponent>();

	auto candleTransComp = e->getComponent<TransformComponent>();
	auto parentTransComp = e->getParent()->getComponent<TransformComponent>();
	/* TODO: Raycast and see if the hit location is ground within x units */
	if ( !candleComp->isCarried() ) {
		if ( candleComp->getIsLit() ) {
			candleTransComp->removeParent();
			glm::vec3 dir = glm::vec3(1.0f, 0.f, 1.0f);// TODO: parentTransComp->getForward()
			candleTransComp->setTranslation(parentTransComp->getTranslation() + dir);
			ECS::Instance()->getSystem<UpdateBoundingBoxSystem>()->update(0.0f);
		} else {
			candleComp->toggleCarried();
		}
	} else if ( candleComp->isCarried() ) {
		if ( glm::length(parentTransComp->getTranslation() - candleTransComp->getTranslation()) < 2.0f || !candleComp->getIsLit() ) {
			candleTransComp->setTranslation(glm::vec3(0.f, 2.0f, 0.f));
			candleTransComp->setParent(parentTransComp);
		} else {
			candleComp->toggleCarried();
		}
	}
}

void CandleSystem::init(GameState* gameStatePtr) {

	this->setGameStatePtr(gameStatePtr);
}
