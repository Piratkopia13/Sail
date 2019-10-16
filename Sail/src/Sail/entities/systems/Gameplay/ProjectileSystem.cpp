#include "pch.h"
#include "ProjectileSystem.h"
#include "Sail/entities/components/ProjectileComponent.h"
#include "Sail/entities/components/CandleComponent.h"
#include "Sail/entities/components/CollisionComponent.h"
#include "Sail/Application.h"

ProjectileSystem::ProjectileSystem() {
	// TODO: System owner should check if this is correct
	registerComponent<ProjectileComponent>(true, true, true);
	registerComponent<CollisionComponent>(true, true, false);
	registerComponent<CandleComponent>(false, true, true);
}

ProjectileSystem::~ProjectileSystem() {

}

void ProjectileSystem::update(float dt) {
	for (auto& e : entities) {
		CollisionComponent* collisionComp = e->getComponent<CollisionComponent>();
		auto projectileCollisions = collisionComp->collisions;
		for (auto& collision : projectileCollisions) {
			// TODO: Replace with some "layer-id" check rather than doing a string check
			/*if (collision.entity->getName().substr(0U, 4U) == "Map_") {
				Octree::RayIntersectionInfo tempInfo;
				//m_octree->getRayIntersection(m_cam.getPosition(), m_cam.getDirection(), &tempInfo);
				if (tempInfo.closestHitIndex != -1) {
					// size (the size you want) = 0.3
					// halfSize = (1 / 0.3) * 0.5 = 1.667
					Application::getInstance()->getRenderWrapper()->getCurrentRenderer()->submitDecal(
						collision./*m_cam.getPosition() + m_cam.getDirection() * tempInfo.closestHit, 
						glm::identity<glm::mat4>(), 
						glm::vec3(1.667f));
				}
			}*/

			if (collision.entity->hasComponent<CandleComponent>()) {
				// TODO: Consume da waterball (smök)
				collision.entity->getComponent<CandleComponent>()->hitWithWater(e->getComponent<ProjectileComponent>()->m_damage);
			}
		}
	}
}