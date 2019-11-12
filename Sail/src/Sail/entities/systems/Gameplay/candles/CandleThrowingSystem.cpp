#include "pch.h"
#include "CandleThrowingSystem.h"

#include "Sail/entities/components/Components.h"
#include "Network/NWrapperSingleton.h"
#include "Sail/utils/GameDataTracker.h"
#include "Sail/netcode/NetworkedStructs.h"

#include "Sail/entities/ECS.h"
#include "Sail/entities/systems/physics/UpdateBoundingBoxSystem.h"

CandleThrowingSystem::CandleThrowingSystem() {
	registerComponent<ThrowingComponent>(true, true, true);
	registerComponent<TransformComponent>(true, true, true);
	registerComponent<MovementComponent>(true, true, true);
	registerComponent<NetworkSenderComponent>(false, true, false);

	registerComponent<AnimationComponent>(false, true, true);
	registerComponent<CollisionComponent>(false, true, true);
	registerComponent<CandleComponent>(false, true, true);
}

CandleThrowingSystem::~CandleThrowingSystem() {}

void CandleThrowingSystem::setOctree(Octree* octree) {
	m_octree = octree;
}

void CandleThrowingSystem::update(float dt) {
	for (auto& e : entities) {
		auto throwC = e->getComponent<ThrowingComponent>();

		// TODO: To be removed once we have animations for it
		auto maxBack = glm::vec3(0.f, 0.f, -0.5f);
		auto maxForward = glm::vec3(0.f, 0.f, 0.0f);

		if (throwC->isDropping) {
			throwC->dropTimer += dt;
			if (throwC->dropTimer > DROP_ANIMATION_LENGTH) {
				throwC->isDropping = false;
				throwC->dropTimer = 0.f;
			}
		} else if (throwC->wasChargingLastFrame || throwC->isThrowing) {

			if (throwC->isCharging) {
				// Charging
				throwC->chargeTime += dt;
			
			} else if (throwC->isThrowing) {
				if (throwC->throwingTimer == 0.f && throwC->chargeTime >= throwC->chargeToThrowThreshold) {
					// Send start throw event
					EventDispatcher::Instance().emit(StartThrowingEvent(e->getComponent<NetworkReceiverComponent>()->m_id));
				}

				throwC->throwingTimer += dt;
				// Just drop it
				if (throwC->chargeTime < throwC->chargeToThrowThreshold) {
					throwC->isThrowing = false;
					throwC->isDropping = true;
					throwC->doThrow = true;
					// Begin drop animation
				} else if (throwC->throwingTimer < CHARGE_AND_THROW_ANIM_LENGTH) {
					// Play charge & throw animation

					/*auto translation = throwC->chargeTime / throwC->maxChargingTime * maxBack +
						throwC->throwingTimer / CHARGE_AND_THROW_ANIM_LENGTH * (maxForward - maxBack);
					for (auto& child : e->getChildEntities()) {
						if (child->hasComponent<CandleComponent>()) {
							child->getComponent<TransformComponent>()->translate(translation);
							continue;
						}
					}*/
				} else {
					// Do the throw
					throwC->isThrowing = false;
					throwC->doThrow = true;
					/*auto translation = /*throwC->chargeTime / throwC->maxChargingTime * maxBack +
						throwC->throwingTimer / CHARGE_AND_THROW_ANIM_LENGTH * (maxForward - maxBack);*/
				}
			}

		}

		if (throwC->doThrow) {
			for (auto& child : e->getChildEntities()) {
				if (child->hasComponent<CandleComponent>()) {
					// Time to throw
					auto transC = child->getComponent<TransformComponent>();
					auto moveC = child->getComponent<MovementComponent>();

					// Remove the candle from players hand
					auto throwPos = glm::vec3(transC->getMatrixWithUpdate()[3]);
					auto parTrans = e->getComponent<TransformComponent>();
					auto parTranslation = parTrans->getTranslation();
					transC->removeParent();
					transC->setRotations(glm::vec3{0.f,0.f,0.f});
					e->getComponent<AnimationComponent>()->rightHandEntity = nullptr;
					//transC->setTranslation(throwPos);

					// Set velocity and things
					throwC->direction = Application::getInstance()->getCurrentCamera()->getDirection();//glm::normalize(-e->getComponent<TransformComponent>()->getForward()/*throwC->direction*/);

					// Making sure the torch isn't dropped inside an object
					auto rayFrom = parTranslation;
					rayFrom.y = throwPos.y;
					Octree::RayIntersectionInfo rayInfo;
					auto rayDir = throwPos - rayFrom;
					auto rayDirNorm = glm::normalize(rayDir);
					m_octree->getRayIntersection(rayFrom, rayDirNorm, &rayInfo, e->getParent(), 0.1f);
					if (!throwC->isDropping) {
						throwPos += throwC->direction * 0.1f;
					}
					if (rayInfo.closestHit < glm::length(rayDir)) {
						throwPos = rayFrom + (rayInfo.closestHit - 0.1f) * rayDirNorm;
					}

					// Set initial throw position
					transC->setTranslation(throwPos);

					// Throw the torch
					moveC->velocity = throwC->direction * throwC->throwingTimer * throwC->throwChargeMultiplier + e->getComponent<MovementComponent>()->velocity;
					moveC->constantAcceleration = glm::vec3(0.f, -9.82f, 0.f);
					// Can be used once the torch light can be set inside the torch instead of on the top of it, LEAVE THIS CODE HERE!
					//throwC->direction.y = 0.f;
					//auto rotationAxis = glm::cross(glm::normalize(throwC->direction), glm::vec3(0.f, 1.f, 0.f));
					//transC->setRotations(glm::angleAxis(glm::radians(-89.5f), rotationAxis));
					child->addComponent<CollisionComponent>(true);
					ECS::Instance()->getSystem<UpdateBoundingBoxSystem>()->update(0.0f);

					// Send end throw event
					if (throwC->throwingTimer >= CHARGE_AND_THROW_ANIM_LENGTH) {
						EventDispatcher::Instance().emit(StopThrowingEvent(e->getComponent<NetworkReceiverComponent>()->m_id));
					}

					// Reset values
					throwC->chargeTime = 0.f;
					throwC->throwingTimer = 0.f;
					throwC->doThrow = false;
					child->getComponent<CandleComponent>()->isCarried = false;


					continue;
				}
			}
		}

		throwC->wasChargingLastFrame = throwC->isCharging;
	}
}

void CandleThrowingSystem::throwCandle(Entity* e) {}
