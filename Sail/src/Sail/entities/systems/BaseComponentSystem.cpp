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

const std::bitset<MAX_NUM_COMPONENTS_TYPES>& BaseComponentSystem::getRequiredComponentTypes() const {
	return requiredComponentTypes;
}

const std::bitset<MAX_NUM_COMPONENTS_TYPES>& BaseComponentSystem::getReadBitMask() const {
	return readBits;
}

const std::bitset<MAX_NUM_COMPONENTS_TYPES>& BaseComponentSystem::getWriteBitMask() const {
	return writeBits;
}
