#include "pch.h"
#include "BaseComponentSystem.h"
#include "..//Entity.h"

bool BaseComponentSystem::addEntity(Entity* entity) {
	for (auto e : entities) {
		if (e->getID() == entity->getID()) {
			return false;
		}
	}
	entities.push_back(entity);
	return true;
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

const unsigned int BaseComponentSystem::getReadBitMask() const {
	return readBits;
}

const unsigned int BaseComponentSystem::getWriteBitMask() const {
	return writeBits;
}
