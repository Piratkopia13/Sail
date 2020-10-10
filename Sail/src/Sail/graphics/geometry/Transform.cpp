#include "pch.h"
#include "Transform.h"

Transform::Transform(const glm::mat4& transformationMatrix) {
	setMatrix(transformationMatrix);
}

Transform::Transform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale)
	: m_translation(translation)
	, m_rotation(rotation)
	, m_scale(scale)
	, m_transformMatrix(1.0f)
	, m_matNeedsUpdate(true)
{ }

Transform::~Transform() {
}

void Transform::translate(const glm::vec3& move) {
	m_translation += move;
	m_matNeedsUpdate = true;
}

void Transform::translate(const float x, const float y, const float z) {
	m_translation += glm::vec3(x, y, z);
	m_matNeedsUpdate = true;
}

void Transform::scale(const float factor) {
	m_scale *= factor;
	m_matNeedsUpdate = true;
}

void Transform::scale(const glm::vec3& scale) {
	m_scale *= scale;
	m_matNeedsUpdate = true;
}

void Transform::rotate(const glm::vec3& rotation) {
	m_rotation += rotation;
	m_matNeedsUpdate = true;
}

void Transform::rotate(const float x, const float y, const float z) {
	m_rotation += glm::vec3(x, y, z);
	m_matNeedsUpdate = true;
}

void Transform::rotateAroundX(const float radians) {
	m_rotation.x += radians;
	m_matNeedsUpdate = true;
}

void Transform::rotateAroundY(const float radians) {
	m_rotation.y += radians;
	m_matNeedsUpdate = true;
}

void Transform::rotateAroundZ(const float radians) {
	m_rotation.z += radians;
	m_matNeedsUpdate = true;
}

void Transform::setTranslation(const glm::vec3& translation) {
	m_translation = translation;
	m_matNeedsUpdate = true;
}

void Transform::setTranslation(const float x, const float y, const float z) {
	m_translation = glm::vec3(x, y, z);
	m_matNeedsUpdate = true;
}

void Transform::setRotations(const glm::vec3& rotations) {
	m_rotation = rotations;
	m_matNeedsUpdate = true;
}

void Transform::setRotations(const float x, const float y, const float z) {
	m_rotation = glm::vec3(x, y, z);
	m_matNeedsUpdate = true;
}

void Transform::setScale(const float scale) {
	m_scale = glm::vec3(scale, scale, scale);
	m_matNeedsUpdate = true;
}

void Transform::setScale(const float x, const float y, const float z) {
	m_scale = glm::vec3(x, y, z);
	m_matNeedsUpdate = true;
}

void Transform::setScale(const glm::vec3& scale) {
	m_scale = scale;
	m_matNeedsUpdate = true;
}

void Transform::setMatrix(const glm::mat4& newMatrix) {
	m_transformMatrix = newMatrix;
	glm::vec3 tempSkew;
	glm::vec4 tempPerspective;
	glm::quat tempRotation;
	glm::decompose(newMatrix, m_scale, tempRotation, m_translation, tempSkew, tempPerspective);
	// TODO: Check that rotation is valid
	m_rotation = glm::eulerAngles(tempRotation);

	m_matNeedsUpdate = false;
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
		updateMatrix();
		m_matNeedsUpdate = false;
	}

	return m_transformMatrix;
}

void Transform::updateMatrix() {
	m_transformMatrix = glm::mat4(1.0f);
	m_transformMatrix = glm::translate(m_transformMatrix, m_translation);
	m_transformMatrix = glm::rotate(m_transformMatrix, m_rotation.x, glm::vec3(1.f, 0.f, 0.f));
	m_transformMatrix = glm::rotate(m_transformMatrix, m_rotation.y, glm::vec3(0.f, 1.f, 0.f));
	m_transformMatrix = glm::rotate(m_transformMatrix, m_rotation.z, glm::vec3(0.f, 0.f, 1.f));
	m_transformMatrix = glm::scale(m_transformMatrix, m_scale);
}