#include "pch.h"
#include "ProjectileSystem.h"
#include "Sail/entities/components/ProjectileComponent.h"
#include "Sail/entities/components/CandleComponent.h"
#include "Sail/entities/components/CollisionComponent.h"
#include "Sail/entities/components/MovementComponent.h"
#include "Sail/entities/components/LocalOwnerComponent.h"
#include "Network/NWrapperSingleton.h"
#include "Sail/Application.h"
#include "API/DX12/renderer/DX12RaytracingRenderer.h"

// The likelihood that a projectile gets destroyed if it collides with at least one other entity this tick.
constexpr float DESTRUCTION_PROBABILITY = 0.3f;

ProjectileSystem::ProjectileSystem() {
	// TODO: System owner should check if this is correct
	registerComponent<ProjectileComponent>(true, true, true);
	registerComponent<CollisionComponent>(true, true, false);
	registerComponent<MovementComponent>(true, true, true);
	registerComponent<CandleComponent>(false, true, true);
	registerComponent<NetworkReceiverComponent>(false, true, false);
	registerComponent<LocalOwnerComponent>(false, true, false);

	float splashSize = 0.14f;
	m_projectileSplashSize = (1.f / splashSize) / 2.f;
}

ProjectileSystem::~ProjectileSystem() {

}

void ProjectileSystem::update(float dt) {
	for (auto& e : entities) {
		CollisionComponent*  collisionComp = e->getComponent<CollisionComponent>();
		ProjectileComponent* projComp      = e->getComponent<ProjectileComponent>();
		auto projectileCollisions = collisionComp->collisions;
		
		bool collidedThisTick = false;

		for (auto& collision : projectileCollisions) {
			collidedThisTick = true;
			
			// Check if a decal should be created
			// TODO: Replace name with some "layer-id" check rather than doing a string check
			if (glm::length(e->getComponent<MovementComponent>()->oldVelocity) > 0.7f
				&& (collision.entity->getName().substr(0U, 4U) == "Map_" || collision.entity->getName().substr(0U, 7U) == "Clutter")) {

				// Calculate rotation matrix used when placing a decal at the intersection
				//glm::mat4 rotMat = glm::rotate(glm::identity<glm::mat4>(), Utils::fastrand() * 3.14f, glm::vec3(0.0f, 0.0f, 1.0f));

				// Place water point at intersection position
				Application::getInstance()->getRenderWrapper()->getCurrentRenderer()->submitWaterPoint(collision.intersectionPosition);

				projComp->timeSinceLastDecal = 0.f;
			}

			//If projectile collided with a candle and the local player owned the projectile
			if (collision.entity->hasComponent<CandleComponent>() && e->hasComponent<LocalOwnerComponent>()) {
				CandleComponent* cc = collision.entity->getComponent<CandleComponent>();
				

				// If that candle isn't our own
				if (!(collision.entity->hasComponent<LocalOwnerComponent>() && cc->isCarried) && !cc->wasHitByMeThisTick) {
					cc->wasHitByMeThisTick = true;
#ifdef DEVELOPMENT
					if (!collision.entity->getParent() || !collision.entity->getParent()->hasComponent<NetworkReceiverComponent>()) {
						SAIL_LOG_WARNING("Projectile hit player who doesn't have a NetworkReceiverComponent\n");
						break;
					}
#endif
					//Inform the host about the hit.( in case you are host this will broadcast to everyone else)
					NWrapperSingleton::getInstance().queueGameStateNetworkSenderEvent(
						Netcode::MessageType::WATER_HIT_PLAYER,
						SAIL_NEW Netcode::MessageWaterHitPlayer{
							collision.entity->getParent()->getComponent<NetworkReceiverComponent>()->m_id
						}
					);
				}
			}

		}

		// The projectile owner is responsible for destroying their own projectiles
		if (collidedThisTick && !e->isAboutToBeDestroyed() && e->hasComponent<LocalOwnerComponent>() && Utils::rnd() < DESTRUCTION_PROBABILITY) {
			e->getComponent<NetworkSenderComponent>()->addMessageType(Netcode::MessageType::DESTROY_ENTITY);
			e->queueDestruction();
		}

		projComp->timeSinceLastDecal += dt;
	}
}