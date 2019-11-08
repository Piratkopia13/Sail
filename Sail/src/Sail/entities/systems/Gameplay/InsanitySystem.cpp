#include "pch.h"
#include "InsanitySystem.h"
#include "Sail/entities/components/InsanityComponent.h"
#include "Sail/entities/components/CandleComponent.h"
#include "Sail/entities/components/TransformComponent.h"
#include "Sail/entities/Entity.h"
#include "Network/NWrapperSingleton.h"

InsanitySystem::InsanitySystem() {
	registerComponent<InsanityComponent>(true, true, true);
	registerComponent<TransformComponent>(true, true, false);
	registerComponent<CandleComponent>(false, true, true);
}

InsanitySystem::~InsanitySystem() {

}

void InsanitySystem::update(float dt) {
	bool isHost = NWrapperSingleton::getInstance().isHost();
	if (!isHost)
		return;

	for (auto& e : entities) {
		InsanityComponent* ic = e->getComponent<InsanityComponent>();
		TransformComponent* tc = e->getComponent<TransformComponent>();
		
		CandleComponent* cc = nullptr;
		TransformComponent* c_tc = nullptr;
		Entity* candle_entity = nullptr;

		for (auto& child : e->getChildEntities()) {
			if ((cc = child->getComponent<CandleComponent>()) && (c_tc = child->getComponent<TransformComponent>())) {
				candle_entity = child;
				break;
			}
		}

		if (candle_entity) {
			float dist;
			if (cc->isCarried) {
				dist = -4;
			} else {
				dist = glm::distance(tc->getTranslation(), c_tc->getTranslation());
			}

			ic->insanityValue -= (dist - 1) * dt * 0.5;

			ic->insanityValue = std::clamp(ic->insanityValue, m_minInsanity, m_maxInsanity);
		}
	}
}
