#include "pch.h"
#include "Entity.h"
#include "ECS.h"

#include "components/TransformComponent.h"

static int s_id = 0;

Entity::SPtr Entity::Create(ECS* ecs, const std::string& name) {
	Entity::SPtr temp = std::make_shared<Entity>(name);
	temp->m_ecs = ecs;
	return temp;
}

void Entity::addToSystems() {
	m_ecs->addEntityToSystems(this);
}

void Entity::removeFromSystems() {
	m_ecs->removeEntityFromSystems(this);
}

void Entity::setECSIndex(int index) {
	m_ECSIndex = index;
}

int Entity::getECSIndex() const {
	return m_ECSIndex;
}

Entity::Entity(const std::string& name) : m_name(name) {
	m_id = s_id++;
	m_ECSIndex = -1;
}

Entity::~Entity() {

}

bool Entity::hasComponents(std::bitset<MAX_NUM_COMPONENTS_TYPES> componentTypes) const {
	return (m_componentTypes & componentTypes).any();
}

bool Entity::isAboutToBeDestroyed() const {
	return m_destructionQueued;
}

void Entity::queueDestruction() {
	m_destructionQueued = true;
	m_ecs->queueDestructionOfEntity(this);
}


// TODO: should only be able to be called on entities with m_destructionQueued == true
void Entity::removeAllComponents() {
	m_components.clear();
	removeFromSystems();
}

void Entity::addChildEntity(Entity::SPtr child) {
	m_children.push_back(child);

	auto transComp = getComponent<TransformComponent>();
	if ( transComp ) {
		auto childTransComp = child->getComponent<TransformComponent>();
		if ( childTransComp ) {
			childTransComp->setParent(transComp);
		}
	}
}

void Entity::removeChildEntity(Entity::SPtr toRemove) {
	m_children.erase(std::find(m_children.begin(), m_children.end(), toRemove));
}

std::vector<Entity::SPtr>& Entity::getChildEntities() {
	return m_children;
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
