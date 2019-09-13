#include "pch.h"
#include "Entity.h"
#include "ECS.h"

static int s_id = 0;

Entity::SPtr Entity::Create(ECS* ecs, const std::string& name) {
	Entity::SPtr temp = std::make_shared<Entity>(name);
	temp->m_ecs = ecs;
	return temp;
}

void Entity::addToSystems() {
	m_ecs->addEntityToSystems(this);
}

void Entity::removeFromSystems()
{
	m_ecs->removeEntityFromSystems(this);
}

Entity::Entity(const std::string& name) : m_name(name) {
	m_id = s_id++;
}

Entity::~Entity() {

}

bool Entity::hasComponent(int id) const
{
	return (m_components.find(id) != m_components.end());
}

void Entity::removeAllComponents() {
	m_components.clear();
	removeFromSystems();
}

void Entity::setName(const std::string& name) {
	m_name = name;
}

const std::string& Entity::getName() const {
	return m_name;
}

int Entity::getID() const {
	return m_id;
}
