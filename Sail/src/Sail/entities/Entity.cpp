#include "pch.h"
#include "Entity.h"

Entity::SPtr Entity::Create(const std::string& name) {
	static size_t counter = 0;
	return std::make_shared<Entity>(counter++, name);
}

Entity::Entity(const size_t& id, const std::string& name)
	: m_uniqueID(id),
	m_name(name)
{

}

Entity::~Entity() {

}

void Entity::setName(const std::string& name) {
	m_name = name;
}

const std::string& Entity::getName() const {
	return m_name;
}

const size_t& Entity::getUniqueID() const {
	return m_uniqueID;
}
