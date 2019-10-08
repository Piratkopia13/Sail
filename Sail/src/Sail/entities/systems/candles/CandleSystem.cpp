#include "pch.h"
#include "CandleSystem.h"

#include "Sail/entities/components/LightComponent.h"
#include "Sail/entities/components/CandleComponent.h"
#include "Sail/entities/components/PhysicsComponent.h"
#include "Sail/entities/components/TransformComponent.h"

#include "Sail/graphics/camera/CameraController.h"

#include "Sail/Application.h"

CandleSystem::CandleSystem() : BaseComponentSystem() {
	// TODO: System owner should check if this is correct
	registerComponent<CandleComponent>(true, true, true);
	registerComponent<TransformComponent>(true, true, false);
	registerComponent<LightComponent>(true, true, true);
}

CandleSystem::~CandleSystem() {

}

void CandleSystem::setPlayerEntityID(int entityID) {
	m_playerEntityID = entityID;
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
	for (auto e : entities) {

		auto candle = e->getComponent<CandleComponent>();

		// Remove light from candles that were hit by projectiles
		if (m_isHit) {
			m_invincibleTimer -= dt;

			if (m_invincibleTimer < 0.0f) {
				m_isHit = false;
				m_invincibleTimer = INVINCIBLE_DURATION;
				candle->resetHitByWater();
			}
		}

		else if (candle->wasHitByWater() && m_health > 0.0f) {
			m_isHit = true;
			m_health -= candle->getDamageTakenLastHit();

			if (m_health <= 0.0f) {
				m_health = 0.0f;
				candle->setIsAlive(false);

				// Check if the extinguished candle is owned by the player
				// If so, dispatch an event (received by GameState for now)
				if (candle->getOwner() == m_playerEntityID) {
					Application::getInstance()->dispatchEvent(Event(Event::Type::PLAYER_CANDLE_HIT));
				}
			}
		}
		else if (!candle->getIsAlive() && candle->getDownTime() >= 4.0f /* Relight the candle every 5 seconds (should probably be removed later) */) {
			candle->setIsAlive(true);
			m_health = MAX_HEALTH;
			candle->resetDownTime();
			candle->resetDoActivate();
		} else if (!candle->getIsAlive()) {
			candle->addToDownTime(dt);
		}

		// COLOR/INTENSITY
		float tempHealthRatio = (m_health / MAX_HEALTH);
		e->getComponent<LightComponent>()->getPointLight().setColor(glm::vec3(tempHealthRatio, tempHealthRatio, tempHealthRatio));
		// POS-OF-LIGHT/-FLAME
		glm::vec3 flamePos = glm::vec3(e->getComponent<TransformComponent>()->getMatrix()[3]) + glm::vec3(0, 0.5f, 0);
		e->getComponent<LightComponent>()->getPointLight().setPosition(flamePos);
	}
}