#include "pch.h"
#include "SprintingSystem.h"
#include "Sail/api/Input.h"
#include "Sail/KeyBinds.h"
#include "Sail/entities/components/SprintingComponent.h"
#include "Sail/entities/components/SpeedLimitComponent.h"
#include "Sail/entities/components/AnimationComponent.h"

SprintingSystem::SprintingSystem() {
	registerComponent<SprintingComponent>(true, true, true);
	registerComponent<AnimationComponent>(false, true, true);
}

SprintingSystem::~SprintingSystem() {}

void SprintingSystem::update(float dt, float alpha) {
	for (auto& e : entities) {
		auto sprintComp = e->getComponent<SprintingComponent>();

		// Downtime of sprint is active
		if (!sprintComp->sprintedLastFrame) {
			sprintComp->downTimer += dt;
			// If we have stopped sprinting and didn't get exhausted, reduce sprint timer
			if (sprintComp->downTimer > RECOVERY_TIME && !sprintComp->exhausted) {
				sprintComp->sprintTimer = glm::clamp(sprintComp->sprintTimer - dt, 0.f, MAX_SPRINT_TIME);
			}
			// else if we haven't sprinted for m_maxDownTime, then reset the sprint 
			else if (sprintComp->downTimer > MAX_SPRINT_DOWN_TIME) {
				sprintComp->downTimer = 0.f;
				sprintComp->sprintTimer = 0.f;
				sprintComp->exhausted = false;
				
			}

			if (e->hasComponent<AnimationComponent>()) {
				e->getComponent<AnimationComponent>()->animationSpeed = 1.f;
			}
		} else {
			sprintComp->downTimer = 0.f;
			sprintComp->sprintTimer += dt;
			if (e->hasComponent<AnimationComponent>()) {
				e->getComponent<AnimationComponent>()->animationSpeed = sprintComp->sprintSpeedModifier;
			}
		}

		if (sprintComp->sprintTimer < MAX_SPRINT_TIME) {
			sprintComp->canSprint = true;
		} else {
			sprintComp->canSprint = false;
			if (sprintComp->sprintedLastFrame) {
				sprintComp->exhausted = true;
			}
		}

		sprintComp->sprintedLastFrame = sprintComp->doSprint && sprintComp->canSprint;
	}
	
}

void SprintingSystem::clean() {}

void SprintingSystem::stop() {}
