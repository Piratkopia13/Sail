#include "pch.h"
#include "BaseComponentSystem.h"
#include "..//Entity.h"

void BaseComponentSystem::addEntity(Entity* entity) {
	for (auto e : m_entities) {
		if (e->getID() == entity->getID()) {
			return;
		}
	}
	m_entities.push_back(entity);
}

void BaseComponentSystem::removeEntity(Entity* entity) {
	for (auto e : m_entities) {
		if (e->getID() == entity->getID()) {
			m_entities.erase(std::remove(m_entities.begin(), m_entities.end(), e), m_entities.end());
			return;
		}
	}
}

const std::vector<int>& BaseComponentSystem::getRequiredComponentTypes() const {
	return m_requiredComponentTypes;
}