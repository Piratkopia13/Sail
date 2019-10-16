#include "pch.h"
#include "CandleSystem.h"

#include "Sail/entities/components/Components.h"

#include "../Sail/src/Network/NWrapperSingleton.h"
#include "Sail/entities/Entity.h"
#include "Sail/graphics/camera/CameraController.h"
#include "Sail/entities/ECS.h"
#include "Sail/entities/systems/physics/UpdateBoundingBoxSystem.h"
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
	for (auto e : entities) {
		if (e->getName() == name) {
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

		if (candle->getIsAlive()) {
			if (candle->getHealth() <= 0.f) {
				candle->setIsLit(false);
				candle->setCarried(true);
				
				// Did current player die?
				if (candle->getNumRespawns() == m_maxNumRespawns) {
					candle->setIsAlive(false);
					LivingCandles--;

					if (NWrapperSingleton::getInstance().isHost()) {
						NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
							Netcode::MessageType::PLAYER_DIED,
							m_playerEntityPtr
						);

						if (LivingCandles <= 1) { // Match IS over
							//TODO: move MATCH_ENDED event to host side and not to client side.
							NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
								Netcode::MessageType::MATCH_ENDED,
								nullptr
							);

							m_gameStatePtr->requestStackPop();
							m_gameStatePtr->requestStackPush(States::EndGame);
						}
					}

					// Check if the extinguished candle is owned by the player
					if (candle->getOwner() == m_playerEntityID) {
						// Match IS NOT over, instead THIS player simply died
							e->getParent()->addComponent<SpectatorComponent>();
							e->getParent()->getComponent<MovementComponent>()->constantAcceleration = glm::vec3(0.f, 0.f, 0.f);
							e->getParent()->removeComponent<GunComponent>();
							e->getParent()->removeAllChildren();
					}
				}
			} else if ((candle->getDoActivate() || candle->getDownTime() >= m_candleForceRespawnTimer) && !candle->getIsLit()) {
				candle->setIsLit(true);
				candle->setHealth(MAX_HEALTH);
				candle->incrementRespawns();
				candle->resetDownTime();
				candle->resetDoActivate();
			} else if (!candle->getIsLit()) {
				candle->addToDownTime(dt);
			}

			if (candle->isCarried() != candle->getWasCarriedLastUpdate()) {
				putDownCandle(e);
			}

			if (candle->getInvincibleTimer() > 0.f) {
				candle->decrementInvincibleTimer(dt);
			}

			// COLOR/INTENSITY
			float cHealth = candle->getHealth();
			cHealth = (cHealth < 0.f) ? 0.f : cHealth;
			float tempHealthRatio = (cHealth / MAX_HEALTH);
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
	if (!candleComp->isCarried()) {
		if (candleComp->getIsLit()) {
			candleTransComp->removeParent();
			glm::vec3 dir = glm::vec3(1.0f, 0.f, 1.0f);// TODO: parentTransComp->getForward()
			candleTransComp->setTranslation(parentTransComp->getTranslation() + dir);
			ECS::Instance()->getSystem<UpdateBoundingBoxSystem>()->update(0.0f);
		} else {
			candleComp->setCarried(true);
		}
	} else {
		if (glm::length(parentTransComp->getTranslation() - candleTransComp->getTranslation()) < 2.0f || !candleComp->getIsLit()) {
			candleTransComp->setTranslation(glm::vec3(0.f, 2.0f, 0.f));
			candleTransComp->setParent(parentTransComp);
		} else {
			candleComp->setCarried(false);
		}
	}
}

void CandleSystem::init(GameState* gameStatePtr) {

	this->setGameStatePtr(gameStatePtr);
}
