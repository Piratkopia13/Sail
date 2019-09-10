#include "pch.h"
#include "ECS.h"

ECS::ECS() {
}

ECS::~ECS() {
}

void ECS::update(float dt) {
	SystemMap::iterator it = m_systems.begin();
	for (; it != m_systems.end(); ++it) {
		it->second->update(dt);
	}
}

Entity::SPtr ECS::createEntity(const std::string& name) {
	m_entities.push_back(Entity::Create(this, name));
	return m_entities.back();
}

void ECS::addEntityToSystems(Entity* entity) {
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