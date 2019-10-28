#include "pch.h"
#include "LifeTimeSystem.h"
#include "Sail/entities/components/LifeTimeComponent.h"
#include "Sail/entities/Entity.h"

LifeTimeSystem::LifeTimeSystem() {
	// TODO: System owner should check if this is correct
	registerComponent<LifeTimeComponent>(true, true, true);
}

LifeTimeSystem::~LifeTimeSystem() {
}

void LifeTimeSystem::update(float dt) {
	LifeTimeComponent* ltc = nullptr;
	for (auto& e : entities) {
		ltc = e->getComponent<LifeTimeComponent>();
		ltc->elapsedTime += dt;

		if (ltc->elapsedTime >= ltc->totalLifeTime) {
			e->queueDestruction();
		}
	}
}