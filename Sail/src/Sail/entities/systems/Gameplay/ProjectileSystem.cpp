#include "pch.h"
#include "ProjectileSystem.h"
#include "Sail/entities/components/ProjectileComponent.h"
#include "Sail/entities/components/CandleComponent.h"
#include "Sail/entities/components/CollisionComponent.h"

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
			if (collision.entity->hasComponent<CandleComponent>()) {
				// TODO: Consume da waterball (sm�k)
				collision.entity->getComponent<CandleComponent>()->hitWithWater(e->getComponent<ProjectileComponent>()->m_damage);
			}
		}
	}
}