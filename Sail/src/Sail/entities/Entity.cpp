#include "pch.h"
#include "Entity.h"

Entity::SPtr Entity::Create(const std::string& name) {
	return std::make_shared<Entity>(name);
}

Entity::Entity(const std::string& name)
	: m_name(name)
	, m_isBeingRendered(false)
{ }

std::unordered_map<int, Component::SPtr>& Entity::getAllComponents() {
	return m_components;
}

Entity::~Entity() { }

bool Entity::removeComponentByID(int id) {
	auto res = m_components.erase(id);
	// Return true if component was successfully removed
	return res != 0;
}

void Entity::setName(const std::string& name) {
	m_name = name;
}

const std::string& Entity::getName() const {
	return m_name;
}

void Entity::setIsBeingRendered(bool value) {
	m_isBeingRendered = value;
}

bool Entity::isBeingRendered() const {
	return m_isBeingRendered;
}
