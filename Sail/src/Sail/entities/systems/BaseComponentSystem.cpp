#include "pch.h"
#include "BaseComponentSystem.h"
#include "..//Entity.h"

void BaseComponentSystem::addEntity(Entity* entity) {
	for (auto e : entities) {
		if (e->getID() == entity->getID()) {
			return;
		}
	}
	entities.push_back(entity);
}

void BaseComponentSystem::removeEntity(Entity* entity) {
	for (auto e : entities) {
		if (e->getID() == entity->getID()) {
			entities.erase(std::remove(entities.begin(), entities.end(), e), entities.end());
			return;
		}
	}
}

const std::vector<int>& BaseComponentSystem::getRequiredComponentTypes() const {
	return requiredComponentTypes;
}
