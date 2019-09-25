#include "pch.h"
#include "CandleSystem.h"

#include "Sail/entities/components/LightComponent.h"
#include "Sail/entities/components/CandleComponent.h"
#include "Sail/entities/components/PhysicsComponent.h"
#include "Sail/entities/components/TransformComponent.h"

#include "Sail/graphics/camera/CameraController.h"

CandleSystem::CandleSystem() : BaseComponentSystem() {
	requiredComponentTypes.push_back(CandleComponent::ID);
	requiredComponentTypes.push_back(TransformComponent::ID); // read-only
	requiredComponentTypes.push_back(LightComponent::ID);
}

CandleSystem::~CandleSystem() {

}

// turn on the light of a specified candle if it doesn't have one already
void CandleSystem::lightCandle(std::string name) {
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
		// Remove light from candles that were hit by projectiles
		if (CandleComponent* c = e->getComponent<CandleComponent>(); c->wasHitByWater()) {
			c->resetHitByWater();
			e->getComponent<LightComponent>()->getPointLight().setColor(glm::vec3(0.0f, 0.0f, 0.0f));
		}
	}
}

void CandleSystem::setPlayerCandle(Entity::SPtr candle) {
	m_playerCandle = candle;
}

void CandleSystem::updatePlayerCandle(CameraController* cam, const float yaw) {
	//moves the candle model and its pointlight to the correct position and rotates it to not spin when the player turns	
	glm::vec3 forward = cam->getCameraDirection();
	forward.y = 0.f;
	forward = glm::normalize(forward);

	glm::vec3 right = glm::cross(glm::vec3(0.f, 1.f, 0.f), forward);
	right = glm::normalize(right);
	glm::vec3 playerToCandle = glm::vec3((forward - right) * 0.2f);
	glm::vec3 candlePos = cam->getCameraPosition() + playerToCandle - glm::vec3(0, 0.35f, 0);
	m_playerCandle->getComponent<TransformComponent>()->setTranslation(candlePos);
	glm::vec3 candleRot = glm::vec3(0.f, glm::radians(-yaw), 0.f);
	m_playerCandle->getComponent<TransformComponent>()->setRotations(candleRot);
	glm::vec3 flamePos = candlePos + glm::vec3(0, 0.37f, 0);
	glm::vec3 plPos = flamePos - playerToCandle * 0.1f;
	m_playerCandle->getComponent<LightComponent>()->getPointLight().setPosition(plPos);
}

// projectiles are presumed to have a PhysicsComponent
void CandleSystem::checkProjectileCollisions(const std::vector<Entity::SPtr> &projectiles) {
	for (auto p : projectiles) {
		auto projectileCollisions = p->getComponent<PhysicsComponent>()->collisions;
		for (auto candle : entities) {
			for (auto collision : projectileCollisions) {
				if (collision.entity == candle) {
					candle->getComponent<CandleComponent>()->hitWithWater();
				}
			}
		}
	}
}
