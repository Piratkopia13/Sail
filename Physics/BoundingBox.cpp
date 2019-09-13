#include "PhysicsPCH.h"
#include "Sail/entities/components/Components.h"

#include "BoundingBox.h"

BoundingBox::BoundingBox() {
	m_drawBoundingBoxes = true;

	m_position = glm::vec3(0.0f);
	m_halfSize = glm::vec3(0.5f);
	m_hasChanged = false;
	for (int i = 0; i < 8; i++) {
		m_corners.push_back(glm::vec3(0.0f));
	}
	updateCorners();
}

BoundingBox::~BoundingBox() {
}

void BoundingBox::updateCorners() {
	m_corners[0] = m_position + glm::vec3(-m_halfSize.x, m_halfSize.y, -m_halfSize.z); //Left Top Close - 0
	m_corners[1] = m_position + glm::vec3(m_halfSize.x, m_halfSize.y, -m_halfSize.z); //Right Top Close - 1
	m_corners[2] = m_position + glm::vec3(-m_halfSize.x, -m_halfSize.y, -m_halfSize.z); //Left Bottom Close - 2
	m_corners[3] = m_position + glm::vec3(m_halfSize.x, -m_halfSize.y, -m_halfSize.z); //Right Bottom Close - 3
	m_corners[4] = m_position + glm::vec3(-m_halfSize.x, m_halfSize.y, m_halfSize.z); //Left Top Far - 4
	m_corners[5] = m_position + glm::vec3(m_halfSize.x, m_halfSize.y, m_halfSize.z); //Right Top Far - 5
	m_corners[6] = m_position + glm::vec3(-m_halfSize.x, -m_halfSize.y, m_halfSize.z); //Left Bottom Far - 6
	m_corners[7] = m_position + glm::vec3(m_halfSize.x, -m_halfSize.y, m_halfSize.z); //Right Bottom Far - 7
}

glm::vec3 BoundingBox::getPosition() const {
	return m_position;
}

glm::vec3 BoundingBox::getHalfSize() const {
	return m_halfSize;
}

const std::vector<glm::vec3>* BoundingBox::getCorners() const {
	return &m_corners;
}

const bool BoundingBox::getChange() {
	bool theChange = m_hasChanged;
	m_hasChanged = false;
	return theChange;
}

void BoundingBox::setPosition(const glm::vec3& position) {
	m_position = position;
	if (m_modelEntity) {
		m_modelEntity->getComponent<TransformComponent>()->setTranslation(m_position);
	}
	updateCorners();
	m_hasChanged = true;
}

void BoundingBox::setHalfSize(const glm::vec3& size) {
	m_halfSize = size;
	if (m_modelEntity) {
		m_modelEntity->getComponent<TransformComponent>()->setScale(m_halfSize * 2.0f);
	}
	updateCorners();
	m_hasChanged = true;
}

void BoundingBox::setModel(Scene* scene, Model* model) {
	if (m_drawBoundingBoxes) {
		if (!m_modelEntity) {
			m_modelEntity = ECS::Instance()->createEntity("Bounding Box Model");
			m_modelEntity->addComponent<ModelComponent>(model);
			m_modelEntity->addComponent<TransformComponent>();
			m_modelEntity->getComponent<TransformComponent>()->setScale(m_halfSize * 2.0f);
			m_modelEntity->getComponent<TransformComponent>()->setTranslation(m_position);
			scene->addEntity(m_modelEntity);
		}
		else {
			m_modelEntity->getComponent<ModelComponent>()->setModel(model);
		}
	}
}

void BoundingBox::hide() {
	if (m_modelEntity) {
		ECS::Instance()->destroyEntity(m_modelEntity);
		m_modelEntity = nullptr;
	}
}