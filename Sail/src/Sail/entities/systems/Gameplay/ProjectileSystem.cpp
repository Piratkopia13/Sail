#include "pch.h"
#include "ProjectileSystem.h"
#include "Sail/entities/components/ProjectileComponent.h"
#include "Sail/entities/components/CandleComponent.h"
#include "Sail/entities/components/PhysicsComponent.h"
#include "Sail/entities/components/TransformComponent.h"

ProjectileSystem::ProjectileSystem() {
	registerComponent<ProjectileComponent>(true, true, true);
	registerComponent<PhysicsComponent>(true, true, false);
	/* Does it really need the transform component? - it's not being used */
	registerComponent<TransformComponent>(true, false, false);
}

ProjectileSystem::~ProjectileSystem() {

}

void ProjectileSystem::update(float dt) {
	for (auto& e : entities) {
		PhysicsComponent* physComp = e->getComponent<PhysicsComponent>();
		auto projectileCollisions = physComp->collisions;
		for (auto& collision : projectileCollisions) {
			if (collision.entity->hasComponent<CandleComponent>()) {
				collision.entity->getComponent<CandleComponent>()->hitWithWater();
			}
		}
	}
}