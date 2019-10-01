#include "pch.h"
#include "ECS.h"
#include "systems/entityManagement/EntityAdderSystem.h"
#include "systems/entityManagement/EntityRemovalSystem.h"

void ECS::addAllQueuedEntities() {
	for (auto& s : m_systems) {
		s.second->addQueuedEntities();
	}
}

ECS::ECS() {
	// Add the special case systems
	m_entityAdderSystem = SAIL_NEW EntityAdderSystem();
	m_entityRemovalSystem = SAIL_NEW EntityRemovalSystem();
}

ECS::~ECS() {
	delete m_entityRemovalSystem;
	delete m_entityAdderSystem;
}

void ECS::stopAllSystems() {
	for (auto &i : m_systems) {
		i.second->stop();
	}
}

void ECS::destroyAllSystems() {
	m_systems.clear();
}

EntityAdderSystem* ECS::getEntityAdderSystem() {
	return m_entityAdderSystem;
}

EntityRemovalSystem* ECS::getEntityRemovalSystem() {
	return m_entityRemovalSystem;
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
	m_entityRemovalSystem->addEntity(entity);
	
	// Might be a poor solution
	m_entityRemovalSystem->addQueuedEntities();
}

void ECS::destroyEntity(const Entity::SPtr entityToRemove) {
	// Find the index of the entity in the vector
	destroyEntity(entityToRemove->getECSIndex());
}

void ECS::destroyEntity(int ecsIndex) {
	// Remove all of the entity's components
	// Also removes it from the systems
	m_entities[ecsIndex]->removeAllComponents();

	// Move the last entity in the vector
	m_entities[ecsIndex] = m_entities.back();

	// Set the index of the moved entity
	m_entities[ecsIndex]->setECSIndex(ecsIndex);

	// Remove the redundant copy of the moved entity
	m_entities.pop_back();
}

void ECS::destroyAllEntities() {
	for (auto& e : m_entities) {
		e->removeAllComponents();
		e->removeFromSystems();
	}
	m_entities.clear();
}

void ECS::addEntityToSystems(Entity* entity) {
	SystemMap::iterator it = m_systems.begin();

	// Check which systems this entity can be placed in
	for ( ; it != m_systems.end(); ++it ) {
		auto componentTypes = it->second->getRequiredComponentTypes();

		// Add this entity to the system
		if ( entity->hasComponents(componentTypes) ) {
			it->second->addEntity(entity);
		}
	}
}

void ECS::removeEntityFromSystems(Entity* entity) {
	SystemMap::iterator it = m_systems.begin();

	for ( ; it != m_systems.end(); ++it ) {
		auto componentTypes = it->second->getRequiredComponentTypes();

		if ( !entity->hasComponents(componentTypes) ) {
			it->second->removeEntity(entity);
		}
	}
}