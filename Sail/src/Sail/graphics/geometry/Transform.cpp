#include "pch.h"
#include "Transform.h"

Transform::Transform(Transform* parent)
	: Transform::Transform() {
	m_parent = parent;
	m_parentUpdated = true;
	if (m_parent)
		m_parent->addChild(this);
}

Transform::Transform(const glm::vec3& translation, Transform* parent)
	: Transform() {
	m_translation = translation;
	m_parent = parent;
	m_parentUpdated = parent;
	if (m_parent)
		m_parent->addChild(this);
}

Transform::Transform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale, Transform* parent)
	: m_translation(translation)
	, m_rotation(rotation)
	, m_scale(scale)
	, m_transformMatrix(1.0f)
	, m_matNeedsUpdate(true)
	, m_parentUpdated(parent)
	, m_parent(parent) {
	if (m_parent)
		m_parent->addChild(this);
}

Transform::~Transform() {
}

void Transform::setParent(Transform* parent) {
	if (m_parent) {
		m_parent->removeChild(this);
	}
	m_parent = parent;
	parent->addChild(this);
	m_parentUpdated = true;
	treeNeedsUpdating();
}

void Transform::removeParent() {
	if (m_parent) {
		m_parent->removeChild(this);
		m_parent = nullptr;
	}
}

void Transform::translate(const glm::vec3& move) {
	m_translation += move;
	m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::translate(const float x, const float y, const float z) {
	m_translation += glm::vec3(x, y, z);
	m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::scale(const float factor) {
	m_scale *= factor;
	m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::scale(const glm::vec3& scale) {
	m_scale *= scale;
	m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::rotate(const glm::vec3& rotation) {
	m_rotation += rotation;
	m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::rotate(const float x, const float y, const float z) {
	m_rotation += glm::vec3(x, y, z);
	m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::rotateAroundX(const float radians) {
	m_rotation.x += radians;
	m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::rotateAroundY(const float radians) {
	m_rotation.y += radians;
	m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::rotateAroundZ(const float radians) {
	m_rotation.z += radians;
	m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setTranslation(const glm::vec3& translation) {
	m_translation = translation;
	m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setTranslation(const float x, const float y, const float z) {
	m_translation = glm::vec3(x, y, z);
	m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setRotations(const glm::vec3& rotations) {
	m_rotation = rotations;
	m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setRotations(const float x, const float y, const float z) {
	m_rotation = glm::vec3(x, y, z);
	m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setScale(const float scale) {
	m_scale = glm::vec3(scale, scale, scale);
	m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setScale(const float x, const float y, const float z) {
	m_scale = glm::vec3(x, y, z);
	m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setScale(const glm::vec3& scale) {
	m_scale = scale;
	m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setMatrix(const glm::mat4& newMatrix) {
	m_localTransformMatrix = newMatrix;
	glm::vec3 tempSkew;
	glm::vec4 tempPerspective;
	glm::quat tempRotation;
	glm::decompose(newMatrix, m_scale, tempRotation, m_translation, tempSkew, tempPerspective);
	// TODO: Doublecheck that rotation is valid
	m_rotation = glm::eulerAngles(tempRotation);

	m_matNeedsUpdate = false;
	treeNeedsUpdating();
}


const glm::vec3& Transform::getTranslation() const {
	return m_translation;
}

const glm::vec3& Transform::getRotations() const {
	return m_rotation;
}

const glm::vec3& Transform::getScale() const {
	return m_scale;
}

glm::mat4 Transform::getMatrix() {
	if (m_matNeedsUpdate) {
		updateLocalMatrix();
		m_matNeedsUpdate = false;
	}
	if (m_parentUpdated || !m_parent) {
		updateMatrix();
		m_parentUpdated = false;
	}

	return m_transformMatrix;
}

glm::mat4 Transform::getLocalMatrix() {
	if (m_matNeedsUpdate) {
		updateLocalMatrix();
		m_matNeedsUpdate = false;
	}
	return m_localTransformMatrix;
}









void Transform::updateLocalMatrix() {
	m_localTransformMatrix = glm::mat4(1.0f);
	m_localTransformMatrix = glm::translate(m_localTransformMatrix, m_translation);
	m_localTransformMatrix = glm::rotate(m_localTransformMatrix, m_rotation.x, glm::vec3(1.f, 0.f, 0.f));
	m_localTransformMatrix = glm::rotate(m_localTransformMatrix, m_rotation.y, glm::vec3(0.f, 1.f, 0.f));
	m_localTransformMatrix = glm::rotate(m_localTransformMatrix, m_rotation.z, glm::vec3(0.f, 0.f, 1.f));
	m_localTransformMatrix = glm::scale(m_localTransformMatrix, m_scale);
}

void Transform::updateMatrix() {
	if (m_parent)
		m_transformMatrix = m_parent->getMatrix() * m_localTransformMatrix;
	else
		m_transformMatrix = m_localTransformMatrix;
}

void Transform::treeNeedsUpdating() {
	m_parentUpdated = true;
	for (Transform* child : m_children) {
		child->treeNeedsUpdating();
	}
}

void Transform::addChild(Transform* transform) {
	m_children.push_back(transform);
}

void Transform::removeChild(Transform* transform) {
	for (int i = 0; i < m_children.size(); i++) {
		if (m_children[i] == transform) {
			m_children[i] = m_children.back();
			m_children.pop_back();
			break;
		}
	}
}
