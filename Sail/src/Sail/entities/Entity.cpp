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

void Entity::addChild(Entity& child) {
	// Both this and the child needs 
	if (!hasComponent<RelationshipComponent>()) {
		addComponent<RelationshipComponent>();
	}
	if (!child.hasComponent<RelationshipComponent>()) {
		child.addComponent<RelationshipComponent>();
	}

	auto& relation = getComponent<RelationshipComponent>();
	auto& childRelation = child.getComponent<RelationshipComponent>();

	// If child is currently parented somewhere else, handle that
	if (childRelation.parent) {
		Entity(childRelation.parent, m_scene).removeChild(child);
	}

	// Add it as a child to this entity
	if (relation.numChildren == 0) {
		relation.first = child;
	} else {
		// Get the last child
		auto curr = Entity(relation.first, m_scene);
		auto& rel = curr.getComponent<RelationshipComponent>();
		while (rel.next) {
			curr = Entity(rel.next, m_scene);
			rel = curr.getComponent<RelationshipComponent>();
		}

		// Add the new child after the currently last child
		rel.next = child;
		// Add the previously last child as the next to last current child
		childRelation.prev = curr;
	}

	childRelation.parent = *this;
	relation.numChildren++;
}

void Entity::removeChild(Entity& child) {
	auto& childRelation = child.getComponent<RelationshipComponent>();

	auto& relation = getComponent<RelationshipComponent>();
	relation.numChildren--;

	if (!childRelation.prev) {
		// This is the first or only child of the last parent, remove it from the parent
		relation.first = {};
	} else {
		// Point child->prev->next to child->next
		auto prev = Entity(childRelation.prev, m_scene);
		auto& rel = prev.getComponent<RelationshipComponent>();

		auto childNext = childRelation.next;
		rel.next = childNext;
	}
	if (childRelation.next) {
		// Point child->next->prev to child->prev
		auto next = Entity(childRelation.next, m_scene);
		auto& rel = next.getComponent<RelationshipComponent>();

		auto childPrev = childRelation.prev;
		rel.prev = childPrev;
	}

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
