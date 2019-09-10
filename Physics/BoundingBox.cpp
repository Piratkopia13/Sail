#include "PhysicsPCH.h"

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
}

void BoundingBox::setSize(glm::vec3 size) {
	m_size = size;
}