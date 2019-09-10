#include "PhysicsPCH.h"
#include "Sail/entities/Entity.h"
#include "Sail/entities/components/Components.h"

#include "BoundingBox.h"

BoundingBox::BoundingBox() {
	m_position = glm::vec3(0.0f);
	m_size = glm::vec3(1.0f);
}

BoundingBox::~BoundingBox() {

}

glm::vec3 BoundingBox::getPosition() const {
	return m_position;
}

glm::vec3 BoundingBox::getSize() const {
	return m_size;
}

void BoundingBox::setPosition(glm::vec3 position) {
	m_position = position;
	m_modelEntity->getComponent<TransformComponent>()->setTranslation(m_position);
}

void BoundingBox::setSize(glm::vec3 size) {
	m_size = size;
	m_modelEntity->getComponent<TransformComponent>()->setScale(m_size);
}

void BoundingBox::setModel(Entity::SPtr modelEntity) {
	m_modelEntity = modelEntity;
}