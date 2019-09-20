#include "pch.h"
#include "PerUpdateRenderObject.h"
#include "Sail/graphics/geometry/Model.h"
#include "Sail/entities/components/TransformComponent.h"
#include "Sail/entities/components/ModelComponent.h"

//FOR DEBUGGING
#include "Sail/graphics/Scene.h"

PerUpdateRenderObject::PerUpdateRenderObject() {
}

PerUpdateRenderObject::PerUpdateRenderObject(TransformComponent* gameObject, ModelComponent* model) : m_model(model->getModel()) {
	createSnapShotFromGameObject(gameObject);
}

PerUpdateRenderObject::PerUpdateRenderObject(Transform* transform, Model* model) : m_model(model) {
	createSnapShotFromGameObject(transform);
}

PerUpdateRenderObject::~PerUpdateRenderObject() 
{}

void PerUpdateRenderObject::setParent(PerUpdateRenderObject* parent) {
	if (m_parent) {
		m_parent->removeChild(this);
	}
	m_parent = parent;
	parent->addChild(this);
}

void PerUpdateRenderObject::removeParent() {
	if (m_parent) {
		m_parent->removeChild(this);
		m_parent = nullptr;
	}
}

void PerUpdateRenderObject::createSnapShotFromGameObject(TransformComponent* object) {
	m_data = object->getTransformFrame();

	// If the object has a parent transform copy that one as well,
	// NOTE: this will cause duplication of some transforms
	if (Transform* parent = object->getParent(); parent) {
		setParent(parent->getRenderTransform());
	}
}

void PerUpdateRenderObject::createSnapShotFromGameObject(Transform* object) {
	m_data = object->getTransformFrame();

	// If the object has a parent transform copy that one as well,
	// NOTE: this will cause duplication of some transforms
	if (Transform * parent = object->getParent(); parent) {
		setParent(parent->getRenderTransform());
	}
}

PerUpdateRenderObject* PerUpdateRenderObject::getParent() const {
	return m_parent;
}

Model* PerUpdateRenderObject::getModel() const {
	return m_model;
}


// returns a transform matrix with linearly interpolated translation, rotation, and scale 
glm::mat4 PerUpdateRenderObject::getMatrix(float alpha) {

	m_transformMatrix = glm::mat4(1.0f);

	// Linear interpolation between the two most recent snapshots
	glm::vec3 trans = (alpha * m_data.m_current.m_translation) + ((1.0f - alpha) * m_data.m_previous.m_translation);
	glm::quat rot = (alpha * m_data.m_current.m_rotationQuat) + ((1.0f - alpha) * m_data.m_previous.m_rotationQuat);
	glm::vec3 scale = (alpha * m_data.m_current.m_scale) + (1.0f - alpha) * m_data.m_previous.m_scale;

	m_localTransformMatrix = glm::mat4(1.0f);
	glm::mat4 transMatrix = glm::translate(m_localTransformMatrix, trans);
	m_rotationMatrix = glm::mat4_cast(rot);
	glm::mat4 scaleMatrix = glm::scale(m_localTransformMatrix, m_data.m_current.m_scale);
	m_localTransformMatrix = transMatrix * m_rotationMatrix * scaleMatrix;
	m_transformMatrix = m_localTransformMatrix;


	return m_transformMatrix;
}

void PerUpdateRenderObject::updateLocalMatrix() {
	m_localTransformMatrix = glm::mat4(1.0f);
	glm::mat4 transMatrix = glm::translate(m_localTransformMatrix, m_data.m_current.m_translation);
	m_rotationMatrix = glm::mat4_cast(m_data.m_current.m_rotationQuat);
	glm::mat4 scaleMatrix = glm::scale(m_localTransformMatrix, m_data.m_current.m_scale);

	m_localTransformMatrix = transMatrix * m_rotationMatrix * scaleMatrix;
}

void PerUpdateRenderObject::updateMatrix() {
	if (m_parent)
		m_transformMatrix = m_parent->getMatrix() * m_localTransformMatrix;
	else
		m_transformMatrix = m_localTransformMatrix;
}

void PerUpdateRenderObject::addChild(PerUpdateRenderObject* transform) {
	m_children.push_back(transform);
}

void PerUpdateRenderObject::removeChild(PerUpdateRenderObject* transform) {
	for (int i = 0; i < m_children.size(); i++) {
		if (m_children[i] == transform) {
			m_children[i] = m_children.back();
			m_children.pop_back();
			break;
		}
	}
}
