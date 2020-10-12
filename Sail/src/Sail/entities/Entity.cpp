#include "pch.h"
#include "Entity.h"
#include "components/Components.h"

Entity::Entity(const ID& handle, Scene* scene) 
	: m_handle(handle)
	, m_scene(scene) 
{ }

std::string Entity::getName() const {
	return getComponent<NameComponent>().name;
}

void Entity::setName(const std::string& name) {
	getComponent<NameComponent>().name = name;
}

bool Entity::isBeingRendered() const {
	return getComponent<IsBeingRenderedComponent>().isBeingRendered;
}

void Entity::setIsBeingRendered(bool isBeingRendered) {
	getComponent<IsBeingRenderedComponent>().isBeingRendered = isBeingRendered;
}

bool Entity::isSelected() const {
	return getComponent<IsSelectedComponent>().isSelected;
}

void Entity::setIsSelected(bool isSelected) {
	getComponent<IsSelectedComponent>().isSelected = isSelected;
}

uint32_t Entity::size() const {
	uint32_t count = 0;
	m_scene->getEnttRegistry().visit(m_handle, [&count](const auto typeID) {
		count++;
	});
	return count;
}

Scene* Entity::getScene() {
	return m_scene;
}
