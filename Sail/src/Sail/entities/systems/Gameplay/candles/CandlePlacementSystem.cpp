#include "pch.h"
#include "CandlePlacementSystem.h"

#include "Sail/entities/components/Components.h"
#include "Network/NWrapperSingleton.h"
#include "Sail/utils/GameDataTracker.h"

#include "Sail/entities/ECS.h"
#include "Sail/entities/systems/physics/UpdateBoundingBoxSystem.h"
#include "glm/gtx/vector_angle.hpp"

#include "Sail/events/EventDispatcher.h"
#include "Sail/events/types/HoldingCandleToggleEvent.h"

CandlePlacementSystem::CandlePlacementSystem() {
	registerComponent<CandleComponent>(true, true, true);
	registerComponent<TransformComponent>(true, true, true);
	registerComponent<NetworkSenderComponent>(false, true, false);

	EventDispatcher::Instance().subscribe(Event::Type::HOLDING_CANDLE_TOGGLE, this);
}

CandlePlacementSystem::~CandlePlacementSystem() {
	EventDispatcher::Instance().unsubscribe(Event::Type::HOLDING_CANDLE_TOGGLE, this);
}

void CandlePlacementSystem::setOctree(Octree* octree) {
	m_octree = octree;
}

void CandlePlacementSystem::update(float dt) {
	for (auto e : entities) {
		auto candle = e->getComponent<CandleComponent>();
		if (candle->isCarried != candle->wasCarriedLastUpdate) {
			putDownCandle(e);
		} else if (!candle->isLit && !candle->isCarried) {
			candle->isCarried = true;
			putDownCandle(e);
		}

		candle->wasCarriedLastUpdate = candle->isCarried;
		static const float candleHeight = 0.37f;
		glm::vec3 flamePos = e->getComponent<TransformComponent>()->getMatrixWithUpdate() * glm::vec4(0, candleHeight, 0, 1);
		e->getComponent<LightComponent>()->getPointLight().setPosition(flamePos);
	}
}

void CandlePlacementSystem::putDownCandle(Entity* e) {
	auto candleComp = e->getComponent<CandleComponent>();
	auto candleTransComp = e->getComponent<TransformComponent>();
	auto parentTransComp = e->getParent()->getComponent<TransformComponent>();

	/* TODO: Raycast and see if the hit location is ground within x units */
	if (!candleComp->isCarried) {
		if (candleComp->isLit) {
			glm::vec3 parentPos = parentTransComp->getTranslation();
			glm::vec3 dir = candleTransComp->getParent()->getForward();
			dir.y = 0.0f;
			dir = glm::normalize(dir) * 0.5f;
			glm::vec3 candleTryPosition = glm::vec3(parentPos.x - dir.x, parentPos.y, parentPos.z - dir.z);

			bool blocked = false;
			glm::vec3 down(0.f, -1.f, 0.f);
			float heightOffsetFromPlayerFeet = 1.f;

			{
				Octree::RayIntersectionInfo tempInfo;
				// Shoot a ray straight down 1 meter ahead of the player to check for floor
				m_octree->getRayIntersection(glm::vec3(candleTryPosition.x, candleTryPosition.y + heightOffsetFromPlayerFeet, candleTryPosition.z), down, &tempInfo, nullptr, 0.01f);
				if (tempInfo.closestHitIndex != -1) {
					float floorCheckVal = glm::angle(tempInfo.info[tempInfo.closestHitIndex].shape->getNormal(), -down);
					// If there's a low angle between the up-vector and the normal of the surface, it can be counted as floor
					bool isFloor = (floorCheckVal < 0.1f) ? true : false;
					if (!isFloor) {
						blocked = true;
					}
					else {
						// Update the height of the candle position
						candleTryPosition.y = candleTryPosition.y + (heightOffsetFromPlayerFeet - tempInfo.closestHit);
					}
				}
				else {
					blocked = true;
				}
			}

			{
				Octree::RayIntersectionInfo tempInfo;
				// Check if the position is visible for the player
				auto playerHead = glm::vec3(parentPos.x, parentPos.y + 1.8f, parentPos.z);
				auto playerHeadToCandle = candleTryPosition - playerHead;
				float eps = 0.0001f;
				m_octree->getRayIntersection(playerHead, glm::normalize(playerHeadToCandle), &tempInfo, nullptr);
				float phtcLength = glm::length(playerHeadToCandle);
				if (tempInfo.closestHit - phtcLength + eps < 0.f) {
					// Can't see the position where we try to place the candle
					blocked = true;
				}
			}

			// Place down the candle if it's not blocked
			if (!blocked) {
				candleTransComp->removeParent();
				candleTransComp->setTranslation(candleTryPosition);
				candleTransComp->setRotations(glm::vec3{ 0.f,0.f,0.f });
				e->getParent()->getComponent<AnimationComponent>()->rightHandEntity = nullptr;

				ECS::Instance()->getSystem<UpdateBoundingBoxSystem>()->update(0.0f);
			}
			else {
				candleComp->isCarried = true;
			}
		}
		else {
			candleComp->isCarried = true;
		}
	}
	else {
		// Pick up the candle
		if (glm::length(parentTransComp->getTranslation() - candleTransComp->getTranslation()) < 2.0f || !candleComp->isLit) {
			candleTransComp->setParent(parentTransComp);
			e->getParent()->getComponent<AnimationComponent>()->rightHandEntity = e;
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
		}
	}
}

bool CandlePlacementSystem::onEvent(const Event& event) {
	auto onHoldingCandleToggle = [](const HoldingCandleToggleEvent& e) {
		auto candleComp = e.candle->getComponent<CandleComponent>();
		auto candleTransComp = e.candle->getComponent<TransformComponent>();

		candleComp->isCarried = e.isHeld;
		candleComp->wasCarriedLastUpdate = e.isHeld;
		if (e.isHeld) {
			candleTransComp->setTranslation(glm::vec3(10.f, 2.0f, 0.f));
			candleTransComp->setParent(e.owner->getComponent<TransformComponent>());

			e.owner->getComponent<AnimationComponent>()->rightHandEntity = e.candle;
		} else {
			candleTransComp->removeParent();
			candleTransComp->setStartTranslation(e.pos);
			candleTransComp->setRotations(glm::vec3{ 0.f,0.f,0.f });
			e.owner->getComponent<AnimationComponent>()->rightHandEntity = nullptr;

			// Might be needed
			ECS::Instance()->getSystem<UpdateBoundingBoxSystem>()->update(0.0f);
		}
	};

	switch (event.type) {
	case Event::Type::HOLDING_CANDLE_TOGGLE: onHoldingCandleToggle((const HoldingCandleToggleEvent&)event); break;
	default: break;
	}

	return true;
}
