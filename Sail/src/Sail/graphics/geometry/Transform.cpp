#include "pch.h"
#include "Transform.h"
#include "PerUpdateRenderObject.h"

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
	//m_data.m_current.m_matNeedsUpdate = true;
	//m_data.m_current.m_parentUpdated = parent;

	m_data.m_previous.m_translation = translation;
	m_data.m_previous.m_rotation = rotation;
	m_data.m_previous.m_rotationQuat = rotation;
	m_data.m_previous.m_scale = scale;
	m_data.m_previous.m_forward = glm::vec3(0.0f);
	m_data.m_previous.m_right = glm::vec3(0.0f);
	m_data.m_previous.m_up = glm::vec3(0.0f);
	//m_data.m_previous.m_matNeedsUpdate = true;
	//m_data.m_previous.m_parentUpdated = parent;

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
	//for (auto& ts : m_transformSnapshots) {
	//	ts.m_parentUpdated = true;
	//}
	//m_data.m_current.m_parentUpdated = true;
	//m_data.m_previous.m_parentUpdated = true;

	treeNeedsUpdating();
	//for (int i = 0; i < SNAPSHOT_BUFFER_SIZE; i++) {
	//	treeNeedsUpdating();
	//}
}

void Transform::removeParent() {
	if (m_parent) {
		m_parent->removeChild(this);
		m_parent = nullptr;
	}
}

// NOTE: Has to be done at the beginning of each update
// copies current state into previous state
void Transform::prepareUpdate() {
	//m_data.m_previous = m_data.m_current;
	m_data.m_previous.m_translation = m_data.m_current.m_translation;
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
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setStartTranslation(const glm::vec3& translation) {
	m_data.m_previous.m_translation = translation;
	//m_data.m_previous.m_matNeedsUpdate = true;
	m_data.m_current.m_translation = translation;
	//m_data.m_current.m_matNeedsUpdate = true;


	/*for (auto& ts : m_transformSnapshots) {
	ts.m_translation = translation;
	ts.m_matNeedsUpdate = true;
	}*/
}

void Transform::translate(const float x, const float y, const float z) {
	m_data.m_current.m_translation += glm::vec3(x, y, z);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::scale(const float factor) {
	m_data.m_current.m_scale *= factor;
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::scale(const glm::vec3& scale) {
	m_data.m_current.m_scale *= scale;
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

/*void Transform::rotate(const glm::vec3& rotation) {
m_rotation += rotation;
m_matNeedsUpdate = true;
treeNeedsUpdating();
}

void Transform::rotate(const float x, const float y, const float z) {
m_rotation += glm::vec3(x, y, z);
m_matNeedsUpdate = true;
treeNeedsUpdating();
}*/
void Transform::rotate(const glm::vec3& rotation) {
	m_data.m_current.m_rotation += rotation;
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::rotate(const float x, const float y, const float z) {
	m_data.m_current.m_rotation += glm::vec3(x, y, z);
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::rotateAroundX(const float radians) {
	m_data.m_current.m_rotation.x += radians;
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::rotateAroundY(const float radians) {
	m_data.m_current.m_rotation.y += radians;
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::rotateAroundZ(const float radians) {
	m_data.m_current.m_rotation.z += radians;
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setTranslation(const glm::vec3& translation) {
	m_data.m_current.m_translation = translation;
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setTranslation(const float x, const float y, const float z) {
	m_data.m_current.m_translation = glm::vec3(x, y, z);
	//m_data.m_current.m_matNeedsUpdate = true;
	m_data.m_current.m_translation = glm::vec3(x, y, z);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setRotations(const glm::vec3& rotations) {
	m_data.m_current.m_rotation = rotations;
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setRotations(const float x, const float y, const float z) {
	m_data.m_current.m_rotation = glm::vec3(x, y, z);
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setScale(const float scale) {
	m_data.m_current.m_scale = glm::vec3(scale, scale, scale	);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setScale(const float x, const float y, const float z) {
	m_data.m_current.m_scale = glm::vec3(x, y, z	);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setScale(const glm::vec3& scale) {
	m_data.m_current.m_scale = scale;
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setForward(const glm::vec3& forward) {
	m_data.m_current.m_forward = glm::vec3(forward.x, forward.y, -forward.z);
	m_data.m_current.m_rotationQuat = glm::rotation(glm::vec3(0.f, 0.f, -1.f), m_data.m_current.m_forward);
	//m_data.m_current.m_matNeedsUpdate = true;
}

//// NOTE: Not used anywhere at the moment
//void Transform::setMatrix(const glm::mat4& newMatrix) {
//	m_localTransformMatrix = newMatrix;
//	glm::vec3 tempSkew;
//	glm::vec4 tempPerspective;
//	glm::quat tempRotation;
//	glm::decompose(newMatrix, 
//		m_data.m_current.m_scale, 
//		tempRotation, 
//		m_data.m_current.m_translation, 
//		tempSkew, tempPerspective);
//	// TODO: Check that rotation is valid
//	m_data.m_current.m_rotation = glm::eulerAngles(tempRotation);
//	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
//
//	//m_data.m_current.m_matNeedsUpdate = false;
//	treeNeedsUpdating();
//}

Transform* Transform::getParent() const {
	return m_parent;
}


// creates a new RenderTransform
PerUpdateRenderObject* Transform::getRenderTransform() const {
	PerUpdateRenderObject* toReturn = SAIL_NEW PerUpdateRenderObject();
	if (m_parent) {
		toReturn->setParent(m_parent->getRenderTransform());
	}
	return toReturn;
}


// Note: returns the translation/rotation/scale that's currently used in update
const glm::vec3& Transform::getTranslation() const {
	return m_data.m_current.m_translation;
}
const glm::vec3& Transform::getRotations() const {
	return m_data.m_current.m_rotation;
}
const glm::vec3& Transform::getScale() const {
	return m_data.m_current.m_scale;
}
//
//const glm::vec3& Transform::getForward() {
//	getMatrix();
//
//	return m_data.m_current.m_forward;
//}

//const glm::vec3& Transform::getRight() {
//	return m_data.m_current.m_right;
//}
//
//const glm::vec3& Transform::getUp() {
//	return m_data.m_current.m_up;
//}


// Not used, all transforms are updated 
void Transform::treeNeedsUpdating() {
	//m_data.m_current.m_parentUpdated = true;
	//for (Transform* child : m_children) {
	//	child->treeNeedsUpdating();
	//}
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
