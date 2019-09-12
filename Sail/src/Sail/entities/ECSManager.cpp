#include "pch.h"
#include "ECS.h"

ECSManager::ECSManager() {
}

ECSManager::~ECSManager() {
}

void ECSManager::update(float dt) {
	SystemMap::iterator it = m_systems.begin();
	for (; it != m_systems.end(); ++it) {
		it->second->update(dt);
	}
}

unsigned ECSManager::nrOfComponentTypes() const {
	return BaseComponent::nrOfComponentTypes();
}

Entity::SPtr ECSManager::createEntity(const std::string& name) {
	m_entities.push_back(Entity::Create(this, name));
	return m_entities.back();
}

void ECSManager::addEntityToSystems(Entity* entity) {
	SystemMap::iterator it = m_systems.begin();
	
	// Check which systems this entity can be placed in
	for (; it != m_systems.end(); ++it) {
		std::vector<int> componentTypes = it->second->getRequiredComponentTypes();
		
		// Check if the entity has all the required components for the system
		bool hasCorrectComponents = true;
		for (auto typeID : componentTypes) {
			if (!entity->hasComponent(typeID)) {
				hasCorrectComponents = false;
				break;
			}
		}

		// Add this entity to the system
		if (hasCorrectComponents) {
			it->second->addEntity(entity);
		}
	}
}

void ECSManager::removeEntityFromSystems(Entity* entity) {
	SystemMap::iterator it = m_systems.begin();

	for (; it != m_systems.end(); ++it) {
		std::vector<int> componentTypes = it->second->getRequiredComponentTypes();

		bool hasCorrectComponents = true;
		for (auto typeID : componentTypes) {
			if (!entity->hasComponent(typeID)) {
				hasCorrectComponents = false;
				break;
			}
		}

		if (!hasCorrectComponents) {
			it->second->removeEntity(entity);
		}
	}
}