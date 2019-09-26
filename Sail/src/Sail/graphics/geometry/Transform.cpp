#include "pch.h"
#include "Transform.h"

Transform::Transform(Transform* parent)
	: Transform::Transform({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, parent) 
{}

Transform::Transform(const glm::vec3& translation, Transform* parent)
	: Transform(translation, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, parent) 
{}

Transform::Transform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale, Transform* parent)
	: m_parent(parent) 
{
	m_data.m_current.m_translation = translation;
	m_data.m_current.m_rotation = rotation;
	m_data.m_current.m_rotationQuat = rotation;
	m_data.m_current.m_scale = scale;
	m_data.m_current.m_forward = glm::vec3(0.0f);
	m_data.m_current.m_right = glm::vec3(0.0f);
	m_data.m_current.m_up = glm::vec3(0.0f);

	m_data.m_previous.m_translation = translation;
	m_data.m_previous.m_rotation = rotation;
	m_data.m_previous.m_rotationQuat = rotation;
	m_data.m_previous.m_scale = scale;
	m_data.m_previous.m_forward = glm::vec3(0.0f);
	m_data.m_previous.m_right = glm::vec3(0.0f);
	m_data.m_previous.m_up = glm::vec3(0.0f);

	m_matNeedsUpdate = true;
	m_parentUpdated = parent;
	m_parentRenderUpdated = parent;
	m_hasChanged = true;

	if (m_parent)
		m_parent->addChild(this);
}

Transform::~Transform() {}

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

// NOTE: Has to be done at the beginning of each update
// Call from PrepareUpdateSystem and nowhere else!
void Transform::prepareUpdate() {
	m_data.m_previous = m_data.m_current;
	m_hasChanged = false;
}


TransformSnapshot Transform::getCurrentTransformState() const {
	return m_data.m_current;
}

TransformSnapshot Transform::getPreviousTransformState() const {
	return m_data.m_previous;
}

TransformFrame Transform::getTransformFrame() const {
	return m_data;
}

void Transform::translate(const glm::vec3& move) {
	m_data.m_current.m_translation += move;
	m_matNeedsUpdate = true;
	m_hasChanged = true;
	treeNeedsUpdating();
}

void Transform::setStartTranslation(const glm::vec3& translation) {
	m_data.m_previous.m_translation = translation;
	m_data.m_current.m_translation = translation;
	m_matNeedsUpdate = true;
}

void Transform::translate(const float x, const float y, const float z) {
	m_data.m_current.m_translation += glm::vec3(x, y, z);
	m_matNeedsUpdate = true;
	m_hasChanged = true;
	treeNeedsUpdating();
}

void Transform::scale(const float factor) {
	m_data.m_current.m_scale *= factor;
	m_matNeedsUpdate = true;
	m_hasChanged = true;
	treeNeedsUpdating();
}

void Transform::scale(const glm::vec3& scale) {
	m_data.m_current.m_scale *= scale;
	m_matNeedsUpdate = true;
	m_hasChanged = true;
	treeNeedsUpdating();
}

void Transform::rotate(const glm::vec3& rotation) {
	m_data.m_current.m_rotation += rotation;
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
	m_matNeedsUpdate = true;
	m_hasChanged = true;
	treeNeedsUpdating();
}

void Transform::rotate(const float x, const float y, const float z) {
	m_data.m_current.m_rotation += glm::vec3(x, y, z);
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
	m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::rotateAroundX(const float radians) {
	m_data.m_current.m_rotation.x += radians;
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
	m_matNeedsUpdate = true;
	m_hasChanged = true;
	treeNeedsUpdating();
}

void Transform::rotateAroundY(const float radians) {
	m_data.m_current.m_rotation.y += radians;
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
	m_matNeedsUpdate = true;
	m_hasChanged = true;
	treeNeedsUpdating();
}

void Transform::rotateAroundZ(const float radians) {
	m_data.m_current.m_rotation.z += radians;
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
	m_matNeedsUpdate = true;
	m_hasChanged = true;
	treeNeedsUpdating();
}

void Transform::setTranslation(const glm::vec3& translation) {
	m_data.m_current.m_translation = translation;
	m_matNeedsUpdate = true;
	m_hasChanged = true;
	treeNeedsUpdating();
}

void Transform::setTranslation(const float x, const float y, const float z) {
	m_data.m_current.m_translation = glm::vec3(x, y, z);
	m_matNeedsUpdate = true;
	m_hasChanged = true;
	treeNeedsUpdating();
}

void Transform::setRotations(const glm::vec3& rotations) {
	m_data.m_current.m_rotation = rotations;
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
	m_matNeedsUpdate = true;
	m_hasChanged = true;
	treeNeedsUpdating();
}

void Transform::setRotations(const float x, const float y, const float z) {
	m_data.m_current.m_rotation = glm::vec3(x, y, z);
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
	m_matNeedsUpdate = true;
	m_hasChanged = true;
	treeNeedsUpdating();
}

void Transform::setScale(const float scale) {
	m_data.m_current.m_scale = glm::vec3(scale, scale, scale	);
	m_matNeedsUpdate = true;
	m_hasChanged = true;
	treeNeedsUpdating();
}

void Transform::setScale(const float x, const float y, const float z) {
	m_data.m_current.m_scale = glm::vec3(x, y, z	);
	m_matNeedsUpdate = true;
	m_hasChanged = true;
	treeNeedsUpdating();
}

void Transform::setScale(const glm::vec3& scale) {
	m_data.m_current.m_scale = scale;
	m_matNeedsUpdate = true;
	m_hasChanged = true;
	treeNeedsUpdating();
}

void Transform::setForward(const glm::vec3& forward) {
	m_data.m_current.m_forward = glm::vec3(forward.x, forward.y, -forward.z);
	m_data.m_current.m_rotationQuat = glm::rotation(glm::vec3(0.f, 0.f, -1.f), m_data.m_current.m_forward);
	m_matNeedsUpdate = true;
}


Transform* Transform::getParent() const {
	return m_parent;
}

const glm::vec3& Transform::getTranslation() const {
	return m_data.m_current.m_translation;
}
const glm::vec3& Transform::getRotations() const {
	return m_data.m_current.m_rotation;
}
const glm::vec3& Transform::getScale() const {
	return m_data.m_current.m_scale;
}

const glm::vec3 Transform::getInterpolatedTranslation(float alpha) const {
	return (alpha * m_data.m_current.m_translation) + ((1.0f - alpha) * m_data.m_previous.m_translation);
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

glm::mat4 Transform::getRenderMatrix(float alpha) {
	// If data hasn't changed use the CPU side transformMatrix
	if (!m_hasChanged && !m_parentRenderUpdated) {
		m_renderMatrix = getMatrix();
	} else {
		// if the data has changed between updates then the matrix will be interpolated every frame
		updateLocalRenderMatrix(alpha);

		updateRenderMatrix(alpha); // if it has then interpolate
		if (m_parentRenderUpdated || !m_parent) {
			updateRenderMatrix(alpha);
			m_parentRenderUpdated = false;
		}
	}

	return m_renderMatrix;
}

void Transform::updateLocalRenderMatrix(float alpha) {
	m_localRenderMatrix = glm::mat4(1.0f);

	// Linear interpolation between the two most recent snapshots
	glm::vec3 trans = (alpha * m_data.m_current.m_translation) + ((1.0f - alpha) * m_data.m_previous.m_translation);
	glm::quat rot = (alpha * m_data.m_current.m_rotationQuat) + ((1.0f - alpha) * m_data.m_previous.m_rotationQuat);
	glm::vec3 scale = (alpha * m_data.m_current.m_scale) + (1.0f - alpha) * m_data.m_previous.m_scale;

	glm::mat4 transMatrix = glm::translate(m_localRenderMatrix, trans);
	glm::mat4 rotationMatrix = glm::mat4_cast(rot);
	glm::mat4 scaleMatrix = glm::scale(m_localRenderMatrix, scale);
	m_localRenderMatrix = transMatrix * rotationMatrix * scaleMatrix;
}

void Transform::updateRenderMatrix(float alpha) {
	if (m_parent) {
		m_renderMatrix = m_parent->getRenderMatrix(alpha) * m_localRenderMatrix;
	} else {
		m_renderMatrix = m_localRenderMatrix;
	}
}

void Transform::updateLocalMatrix() {
	m_localTransformMatrix = glm::mat4(1.0f);

	glm::mat4 transMatrix = glm::translate(m_localTransformMatrix, m_data.m_current.m_translation);
	glm::mat4 rotationMatrix = glm::mat4_cast(m_data.m_current.m_rotationQuat);
	glm::mat4 scaleMatrix = glm::scale(m_localTransformMatrix, m_data.m_current.m_scale);
	m_localTransformMatrix = transMatrix * rotationMatrix * scaleMatrix;
}

void Transform::updateMatrix() {
	if (m_parent) {
		m_transformMatrix = m_parent->getMatrix() * m_localTransformMatrix;
	} else {
		m_transformMatrix = m_localTransformMatrix;
	}
	m_hasChanged = true;
}

void Transform::treeNeedsUpdating() {
	m_parentUpdated = true;
	m_parentRenderUpdated = true;
	m_hasChanged = true;
	for (Transform* child : m_children) {
		child->treeNeedsUpdating();
	}
}

void Transform::addChild(Transform* transform) {
	m_children.push_back(transform);
}

void Transform::removeChild(Transform* Transform) {
	for (int i = 0; i < m_children.size(); i++) {
		if (m_children[i] == Transform) {
			m_children[i] = m_children.back();
			m_children.pop_back();
			break;
		}
	}
}

const bool Transform::getChange() {
	return m_hasChanged;
}