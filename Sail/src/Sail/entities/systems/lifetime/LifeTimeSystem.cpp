#include "pch.h"
#include "LifeTimeSystem.h"
#include "..//..//components/LifeTimeComponent.h"
#include "..//..//Entity.h"

LifeTimeSystem::LifeTimeSystem() {
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