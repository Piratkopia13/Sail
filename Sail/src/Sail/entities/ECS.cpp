#include "pch.h"
#include "ECS.h"

ECS::ECS() {

}

ECS::~ECS() {
}

//void ECS::update(float dt) {
//	SystemMap::iterator it = m_systems.begin();
//	for (; it != m_systems.end(); ++it) {
//		it->second->update(dt);
//	}
//}

unsigned ECS::nrOfComponentTypes() const {
	return BaseComponent::nrOfComponentTypes();
}

Entity::SPtr ECS::createEntity(const std::string& name) {
	m_entities.push_back(Entity::Create(this, name));
	return m_entities.back();
}

void ECS::queueDestructionOfEntity(const Entity::SPtr entity) {
	//Loop through and find entity
	for (auto e : m_entities) {
	//for (unsigned int i = 0; i < m_entities.size(); i++) {
		if (e == entity) { //Entity found
			e->queueDestruction();
			break;
		}
	}
}

void ECS::destroyEntity(const Entity::SPtr entityToRemove) {
	//Loop through and find entity
	for (unsigned int i = 0; i < m_entities.size(); i++) {
		if (m_entities[i] == entityToRemove) { //Entity found
			//Destroy it
			m_entities[i]->removeAllComponents();
			m_entities.erase(m_entities.begin() + i);
			break;
		}
	}
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