#include "pch.h"
#include "CandleSystem.h"

#include "Sail/entities/components/LightComponent.h"
#include "Sail/entities/components/CandleComponent.h"
#include "Sail/entities/components/TransformComponent.h"

CandleSystem::CandleSystem() : BaseComponentSystem() {
	requiredComponentTypes.push_back(CandleComponent::ID);
	requiredComponentTypes.push_back(TransformComponent::ID); // read-only
}

CandleSystem::~CandleSystem() {

}

// add light to specified candle if it doesn't have one already
void CandleSystem::addLightToCandle(std::string name) {
	for (auto e : entities) {
		if (e->getName() == name) {
			if (!e->hasComponent<LightComponent>()) {
				PointLight pl;
				glm::vec3 pos = e->getComponent<TransformComponent>()->getTranslation();
				pl.setColor(glm::vec3(1.f, 1.f, 1.f));
				pl.setPosition(glm::vec3(pos.x, pos.y + 3.1, pos.z));
				pl.setAttenuation(.0f, 0.1f, 0.02f);
				pl.setIndex(0);
				e->addComponent<LightComponent>(pl);
			}
		}
	}
}

// should be updated after collision detection has been done
void CandleSystem::update(float dt) {
	for (auto e : entities) {
		// Remove light from candles that were hit by projectiles
		if (CandleComponent* c = e->getComponent<CandleComponent>(); c->wasHitByWater()) {
			c->resetHitByWater();
			if (e->getComponent<LightComponent>()) {
				e->removeComponent<LightComponent>();
			}
		}
	}
}

void CandleSystem::setPlayerCandle(Entity* candle) {
	m_playerCandle = candle;
}
