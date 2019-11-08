#include "pch.h"
#include "CandlePlacementSystem.h"

#include "Sail/entities/components/Components.h"
#include "Network/NWrapperSingleton.h"
#include "Sail/utils/GameDataTracker.h"

#include "Sail/entities/ECS.h"
#include "Sail/entities/systems/physics/UpdateBoundingBoxSystem.h"
#include "glm/gtx/vector_angle.hpp"

CandlePlacementSystem::CandlePlacementSystem() {
	registerComponent<CandleComponent>(true, true, true);
	registerComponent<TransformComponent>(true, true, true);
	registerComponent<NetworkSenderComponent>(false, true, false);
}

CandlePlacementSystem::~CandlePlacementSystem() {}

void CandlePlacementSystem::setOctree(Octree* octree) {
	m_octree = octree;
}

void CandlePlacementSystem::update(float dt) {
	for (auto e : entities) {
		auto candle = e->getComponent<CandleComponent>();
		if (candle->isCarried != candle->wasCarriedLastUpdate) {
			toggleCandlePlacement(e);
		} else if (!candle->isLit && !candle->isCarried) {
			candle->isCarried = true;
			toggleCandlePlacement(e);
		}

		candle->wasCarriedLastUpdate = candle->isCarried;
		static const float candleHeight = 0.5f;
		glm::vec3 flamePos = e->getComponent<TransformComponent>()->getMatrixWithUpdate() * glm::vec4(0, candleHeight, 0, 1);
		e->getComponent<LightComponent>()->getPointLight().setPosition(flamePos);
	}
}

void CandlePlacementSystem::toggleCandlePlacement(Entity* e) {
	auto candleComp = e->getComponent<CandleComponent>();
	auto candleTransComp = e->getComponent<TransformComponent>();
	auto parentTransComp = e->getParent()->getComponent<TransformComponent>();

	if (candleComp->isCarried) {
		// Pick up the candle
		if (glm::length(parentTransComp->getTranslation() - candleTransComp->getTranslation()) < 2.0f || !candleComp->isLit) {
			candleTransComp->setParent(parentTransComp);
			e->getParent()->getComponent<AnimationComponent>()->rightHandEntity = e;
			e->removeComponent<CollisionComponent>();
		}
		else {
			candleComp->isCarried = false;
		}
	}

	// Only send the message if we actually did something to the candle
	if (candleComp->isCarried != candleComp->wasCarriedLastUpdate) {
		// Inform other players that we've put down our candle
		if (e->getParent()->getComponent<LocalOwnerComponent>()) {
			NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
				Netcode::MessageType::CANDLE_HELD_STATE,
				SAIL_NEW Netcode::MessageCandleHeldState{
					e->getParent()->getComponent<NetworkSenderComponent>()->m_id,
					candleComp->isCarried,
					e->getComponent<TransformComponent>()->getTranslation()
				},
				false // We've already put down our own candle so no need to do it again
			);

			auto senderC = e->getComponent<NetworkSenderComponent>();

			if (candleComp->isCarried) {
				// If we're holding the candle it will be attached to our animation so don't send its
				// position and rotation.
				senderC->removeMessageType(Netcode::MessageType::CHANGE_LOCAL_POSITION);
				senderC->removeMessageType(Netcode::MessageType::CHANGE_LOCAL_ROTATION);
			} else {
				// If we're no longer holding the candle then start sending its position and rotation
				// so that other people will be able to see it when we throw it.
				senderC->addMessageType(Netcode::MessageType::CHANGE_LOCAL_POSITION);
				senderC->addMessageType(Netcode::MessageType::CHANGE_LOCAL_ROTATION);
			}
		}
	}
}
