#include "pch.h"
#include "ECS.h"
#include "systems/Cleanup/EntityRemovalSystem.h"

ECS::ECS() {
	// Add the cleanup system
	m_systems[typeid(EntityRemovalSystem)] = std::make_unique<EntityRemovalSystem>();
}

ECS::~ECS() {
}

unsigned ECS::nrOfComponentTypes() const {
	return BaseComponent::nrOfComponentTypes();
}



Entity::SPtr ECS::createEntity(const std::string& name) {
	m_entities.push_back(Entity::Create(this, name));
	m_entities.back()->setECSIndex(m_entities.size() - 1);
	return m_entities.back();
}

void ECS::queueDestructionOfEntity(Entity* entity) {
	// Add entity to removal system
	m_systems.at(typeid(EntityRemovalSystem)).get()->addEntity(entity);
}

void ECS::destroyEntity(const Entity::SPtr entityToRemove) {
	//Loop through and find entity
	
	// Find the index of the entity in the vector
	int index = entityToRemove->getECSIndex();

	// Remove all of its components
	// Also removes it from the systems
	m_entities[index]->removeAllComponents();

	// Move the last entity in the vector
	m_entities[index] = m_entities.back();

	// Set the index of the moved entity
	m_entities[index]->setECSIndex(index);

	// Remove the redundant copy of the moved entity
	m_entities.pop_back();
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

void ECS::removeEntityFromSystems(Entity* entity) {
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