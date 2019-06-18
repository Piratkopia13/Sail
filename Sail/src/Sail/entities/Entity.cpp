#include "pch.h"
#include "Entity.h"

Entity::SPtr Entity::Create(const std::string& name) {
	return std::make_shared<Entity>(name);
}

Entity::Entity(const std::string& name) 
	: m_name(name)
{

}

Entity::~Entity() {

}

void Entity::setName(const std::string& name) {
	m_name = name;
}

const std::string Entity::getName() const {
	return m_name;
}
