#include "pch.h"
#include "MovementSystem.h"
#include "..//..//components/TransformComponent.h"
#include "..//..//components/MovementComponent.h"
#include "..//..//Entity.h"

MovementSystem::MovementSystem() {
	registerComponent<TransformComponent>(true, true, true);
	registerComponent<MovementComponent>(true, true, true);
}

MovementSystem::~MovementSystem() {
}

void MovementSystem::update(float dt) {
	for (auto& e : entities) {

		TransformComponent* transform = e->getComponent<TransformComponent>();
		MovementComponent* movement = e->getComponent<MovementComponent>();

		// Update velocity
		if (e->getName() == "projectile") {
			int asdf = 3;
		}
		movement->velocity += (movement->constantAcceleration + movement->accelerationToAdd) * dt;

		// Reset additional acceleration
		movement->accelerationToAdd = glm::vec3(0.0f);
		
		// Rotation
		if (movement->rotation != glm::vec3(0.0f)) {
			transform->rotate(movement->rotation * dt);
		}

		// Set initial value which might be changed in CollisionSystem
		movement->updateableDt = dt;
	}
}
