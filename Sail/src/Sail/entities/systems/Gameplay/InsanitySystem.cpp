#include "pch.h"
#include "InsanitySystem.h"
#include "Sail/entities/components/InsanityComponent.h"
#include "Sail/entities/components/CandleComponent.h"
#include "Sail/entities/components/TransformComponent.h"
#include "Sail/entities/components/NetworkReceiverComponent.h"

#include "Sail/entities/Entity.h"
#include "Network/NWrapperSingleton.h"
#include "Sail/events/Event.h"
#include "Sail/events/Events.h"

InsanitySystem::InsanitySystem() {
	registerComponent<InsanityComponent>(true, true, true);
	registerComponent<TransformComponent>(true, true, false);
	registerComponent<CandleComponent>(false, true, true);

	EventDispatcher::Instance().subscribe(Event::Type::INSANITY_SYSTEM_UPDATE_INSANITY, this);
}

InsanitySystem::~InsanitySystem() {
	EventDispatcher::Instance().unsubscribe(Event::Type::INSANITY_SYSTEM_UPDATE_INSANITY, this);
}

void InsanitySystem::update(float dt) {
	bool isHost = NWrapperSingleton::getInstance().isHost();
	
	//Only let host controll insanity for all players.
	if (!isHost) {
		return;
	}

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
			if (ic->insanityValue <= 0) {
				cc->kill(CandleComponent::DamageSource::INSANE, 255);
			}
		}
	}
}

void InsanitySystem::updateInsanityNetworked(Netcode::ComponentID id, float insanity) {
	bool found = false;

	for (auto& e : entities) {
		NetworkReceiverComponent* nc = nullptr;
		InsanityComponent* ic = nullptr;

		if (nc = e->getComponent<NetworkReceiverComponent>()) {
			if (nc->m_id == id) {
				found = true;

				if (ic = e->getComponent<InsanityComponent>()) {
					ic->insanityValue = insanity;
				} else {
					SAIL_LOG_WARNING("Tried to update insanity on an entity without a insanityComponent!\n");
				}
				break;
			}
		}
	}

#ifdef DEVELOPMENT
	if (!found) {
		SAIL_LOG_WARNING("Tried to update insanity on an entity that do not exist!\n");
	}
#endif

}

bool InsanitySystem::onEvent(const Event& event) {
    switch(event.type) {                                                                             
	case Event::Type::INSANITY_SYSTEM_UPDATE_INSANITY:
		const UpdateInsanityEvent& e = static_cast<const UpdateInsanityEvent&>(event);
		updateInsanityNetworked(e.id, e.insanityVal);
		break;
	 }

	return true;
} 