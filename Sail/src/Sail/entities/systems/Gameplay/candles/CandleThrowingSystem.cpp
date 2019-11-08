#include "pch.h"
#include "CandleThrowingSystem.h"

#include "Sail/entities/components/Components.h"
#include "Network/NWrapperSingleton.h"
#include "Sail/utils/GameDataTracker.h"

#include "Sail/entities/ECS.h"
#include "Sail/entities/systems/physics/UpdateBoundingBoxSystem.h"

CandleThrowingSystem::CandleThrowingSystem() {
	registerComponent<ThrowingComponent>(true, true, true);
	registerComponent<TransformComponent>(true, true, true);
	registerComponent<MovementComponent>(true, true, true);
	registerComponent<NetworkSenderComponent>(false, true, false);

	registerComponent<AnimationComponent>(false, true, true);
	registerComponent<CollisionComponent>(false, true, true);
}

CandleThrowingSystem::~CandleThrowingSystem() {}

void CandleThrowingSystem::update(float dt) {
	for (auto& e : entities) {
		auto throwC = e->getComponent<ThrowingComponent>();

		if (throwC->wasChargingLastFrame) {
			if (throwC->isCharging) {
				// Charging
				throwC->chargeTime += dt;
				throwC->chargeTime = glm::min(throwC->chargeTime, throwC->maxChargingTime);
			} else {
				// Time to throw
				auto transC = e->getComponent<TransformComponent>();
				auto moveC = e->getComponent<MovementComponent>();

				// Remove the candle from players hand
				auto playerHand = transC->getTranslation();
				auto parTrans = e->getParent()->getComponent<TransformComponent>()->getTranslation();
				transC->removeParent();
				transC->setRotations(glm::vec3{0.f,0.f,0.f});
				e->getParent()->getComponent<AnimationComponent>()->rightHandEntity = nullptr;
				transC->setTranslation(parTrans + playerHand);
				ECS::Instance()->getSystem<UpdateBoundingBoxSystem>()->update(0.0f);

				// Set velocity and things
				throwC->direction = glm::normalize(throwC->direction);
				transC->translate(throwC->direction);
				e->addComponent<CollisionComponent>();
				moveC->velocity = throwC->direction * throwC->chargeTime * throwC->throwChargeMultiplier + e->getParent()->getComponent<MovementComponent>()->velocity;
				moveC->constantAcceleration = glm::vec3(0.f, -9.82f, 0.f);

				// Reset values
				throwC->chargeTime = 0.f;
				throwC->isCharging = false;
			}
		}

		throwC->wasChargingLastFrame = throwC->isCharging;
	}
}

void CandleThrowingSystem::throwCandle(Entity* e) {}
