#include "pch.h"
#include "EntityRemovalSystem.h"
#include "..//..//ECS.h"

EntityRemovalSystem::EntityRemovalSystem() {
}

EntityRemovalSystem::~EntityRemovalSystem() {
}

void EntityRemovalSystem::update(float dt) {
	for (size_t i = 0; i < entities.size(); i++) {
		ECS::Instance()->destroyEntity(entities[i]->getECSIndex());
	}
	entities.clear();
}