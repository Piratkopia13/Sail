#include "pch.h"
#include "ProjectileSystem.h"
#include "Sail/entities/components/ProjectileComponent.h"
#include "Sail/entities/components/CandleComponent.h"
#include "Sail/entities/components/PhysicsComponent.h"
#include "Sail/entities/components/TransformComponent.h"

ProjectileSystem::ProjectileSystem() {
	requiredComponentTypes.push_back(ProjectileComponent::ID);
	requiredComponentTypes.push_back(PhysicsComponent::ID);
	requiredComponentTypes.push_back(TransformComponent::ID);
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