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

		movement->velocity += movement->acceleration * dt;
		movement->velocity += movement->accelerationToAdd * dt;

		movement->accelerationToAdd = glm::vec3(0.0f);
		
		if (movement->rotation != glm::vec3(0.0f)) {
			transform->rotate(movement->rotation * dt);
		}
	}
}
