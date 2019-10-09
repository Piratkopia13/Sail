#include "pch.h"
#include "BaseComponentSystem.h"
#include "..//Entity.h"

bool BaseComponentSystem::addEntity(Entity* entity) {
	
	// Check if the entity is in the system
	for (auto e : entities) {
		if (e->getID() == entity->getID()) {
			return false;
		}
	}

	// Check if the entity is about to be in the system
	for (auto e : entitiesQueuedToAdd) {
		if (e->getID() == entity->getID()) {
			return false;
		}
	}

	// Queue the adding of the entity 
	entitiesQueuedToAdd.push_back(entity);

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

void BaseComponentSystem::clearEntities() {
	entities.clear();
	entitiesQueuedToAdd.clear();
}

void BaseComponentSystem::addQueuedEntities() {
	entities.insert(entities.end(), entitiesQueuedToAdd.begin(), entitiesQueuedToAdd.end());
	entitiesQueuedToAdd.clear();
}
