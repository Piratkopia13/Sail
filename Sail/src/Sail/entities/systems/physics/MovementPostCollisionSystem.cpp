#include "pch.h"
#include "MovementPostCollisionSystem.h"
#include "..//..//components/TransformComponent.h"
#include "..//..//components/MovementComponent.h"
#include "..//..//Entity.h"
#include "Sail/utils/GameDataTracker.h"

MovementPostCollisionSystem::MovementPostCollisionSystem() {
	registerComponent<TransformComponent>(true, true, true);
	registerComponent<MovementComponent>(true, true, true);
}

MovementPostCollisionSystem::~MovementPostCollisionSystem() {
}

void MovementPostCollisionSystem::update(float dt) {
	for (auto& e : entities) {
		TransformComponent* transform = e->getComponent<TransformComponent>();
		MovementComponent* movement = e->getComponent<MovementComponent>();

		// Update position with velocities after CollisionSystem has potentially altered them
		glm::vec3 translation = (movement->oldVelocity + movement->velocity) * (0.5f * movement->updateableDt);
		if (translation != glm::vec3(0.0f)) {
			transform->translate(translation);
			if (e->getName() == "player") {
				GameDataTracker::getInstance().logDistanceWalked(translation);
			}
		}

		movement->oldVelocity = movement->velocity;
	}
}
