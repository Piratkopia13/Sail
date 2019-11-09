#include "pch.h"
#include "SpeedLimitSystem.h"
#include "..//..//components/MovementComponent.h"
#include "..//..//components/SpeedLimitComponent.h"
#include "Sail/entities/components/SprintingComponent.h"
#include "..//..//Entity.h"

SpeedLimitSystem::SpeedLimitSystem() {
	registerComponent<MovementComponent>(true, true, true);
	registerComponent<SpeedLimitComponent>(true, true, false);
	registerComponent<SprintingComponent>(false, true, false);
}

SpeedLimitSystem::~SpeedLimitSystem() {
}

void SpeedLimitSystem::update() {
	for (auto& e : entities) {
		MovementComponent* movement = e->getComponent<MovementComponent>();
		SpeedLimitComponent* speedLimit = e->getComponent<SpeedLimitComponent>();

		// Retain vertical speed
		const float ySpeed = movement->velocity.y;

		// Calculate horizontal speed
		glm::vec3 newVelocity = glm::vec3(movement->velocity.x, 0.0f, movement->velocity.z);
		const float horizontalSpeedSquared = glm::length2(newVelocity);

		float maxSpeed = speedLimit->maxSpeed;
		// Modify max speed if sprinting
		if (e->hasComponent<SprintingComponent>()) {
			auto sprintComp = e->getComponent<SprintingComponent>();
			if (sprintComp->sprintedLastFrame) {
				maxSpeed = maxSpeed * sprintComp->sprintSpeedModifier;
			}
		}
		// Limit max speed
		if (horizontalSpeedSquared > maxSpeed * maxSpeed) {
			newVelocity = glm::normalize(newVelocity) * speedLimit->maxSpeed;
		}

		// Limit min speed
		if (horizontalSpeedSquared < 0.05f * 0.05f) {	// Retains minimum speed from old code
			newVelocity = glm::vec3(0.0f);
		}

		// Set new velocity while retaining vertical speed
		newVelocity.y = ySpeed;
		movement->velocity = newVelocity;
	}
}
