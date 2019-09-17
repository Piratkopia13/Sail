#include "pch.h"
#include "Transform.h"

std::atomic_uint Transform::s_frameIndex = 0;
UINT Transform::s_updateIndex = 0;
UINT Transform::s_renderIndex = 0;

// STATIC FUNCTIONS

// To be done at the end of each CPU update and nowhere else	
void Transform::IncrementCurrentUpdateIndex() {
	s_frameIndex++;
	s_updateIndex = s_frameIndex.load() % SNAPSHOT_BUFFER_SIZE;
}

// To be done just before render is called
void Transform::UpdateCurrentRenderIndex() {
	s_renderIndex = prevInd(s_frameIndex.load());
}

#ifdef _DEBUG
UINT Transform::GetUpdateIndex() { return s_updateIndex; }
UINT Transform::GetRenderIndex() { return s_renderIndex; }
#endif

// NON-STATIC FUNCTIONS

Transform::Transform(Transform* parent)
	: Transform::Transform({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, parent) 
{}

Transform::Transform(TransformSnapshot current, TransformSnapshot prev) 
	: m_currentState(current), m_previousState(prev) 
{}


Transform::Transform(const glm::vec3& translation, Transform* parent)
	: Transform(translation, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, parent) 
{}

Transform::Transform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale, Transform* parent)
	: m_transformMatrix(1.0f)
	, m_localTransformMatrix(1.0f)
	, m_parent(parent) 
{
	//for (auto& ts : m_transformSnapshots) {
	//	ts.m_translation = translation;
	//	ts.m_rotation = rotation;
	//	ts.m_rotationQuat = rotation;
	//	ts.m_scale = scale;
	//	ts.m_forward = glm::vec3(0.0f);
	//	ts.m_right = glm::vec3(0.0f);
	//	ts.m_up = glm::vec3(0.0f);
	//	ts.m_matNeedsUpdate = true;
	//	ts.m_parentUpdated = parent;
	//}

	m_currentState.m_translation = translation;
	m_currentState.m_rotation = rotation;
	m_currentState.m_rotationQuat = rotation;
	m_currentState.m_scale = scale;
	m_currentState.m_forward = glm::vec3(0.0f);
	m_currentState.m_right = glm::vec3(0.0f);
	m_currentState.m_up = glm::vec3(0.0f);
	m_currentState.m_matNeedsUpdate = true;
	m_currentState.m_parentUpdated = parent;

	m_previousState.m_translation = translation;
	m_previousState.m_rotation = rotation;
	m_previousState.m_rotationQuat = rotation;
	m_previousState.m_scale = scale;
	m_previousState.m_forward = glm::vec3(0.0f);
	m_previousState.m_right = glm::vec3(0.0f);
	m_previousState.m_up = glm::vec3(0.0f);
	m_previousState.m_matNeedsUpdate = true;
	m_previousState.m_parentUpdated = parent;

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
	m_currentState.m_parentUpdated = true;
	m_previousState.m_parentUpdated = true;

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
	m_previousState = m_currentState;
	//m_transformSnapshots[s_updateIndex] = m_transformSnapshots[prevInd(s_updateIndex)];
}

// NOT USED
TransformSnapshot Transform::getCurrentTransformState() const {
	return m_currentState;
}

TransformSnapshot Transform::getPreviousTransformState() const {
	return m_previousState;
}


// Returns a copy with the current and previous snapshots as well as copies of the parents' snapshots
Transform* Transform::getTransformSnapshot() const {
	Transform* toReturn = SAIL_NEW Transform(m_currentState, m_previousState);
	if (m_parent) {
		toReturn->setParent(m_parent->getTransformSnapshot());
	}
	return toReturn;
}

void Transform::copySnapshotFromObject(Transform* object) {
	m_previousState= object->getPreviousTransformState();
	m_currentState = object->getCurrentTransformState();

	// If the object has a parent transform copy that one as well,
	// NOTE: this will cause duplication of some transforms
	if (Transform* parent = object->getParent(); parent) {
		setParent(parent->getTransformSnapshot());
	}
}


void Transform::translate(const glm::vec3& move) {
	//m_transformSnapshots[s_updateIndex].m_translation += move;
	//m_transformSnapshots[s_updateIndex].m_matNeedsUpdate = true;
	m_currentState.m_translation += move;
	m_currentState.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setStartTranslation(const glm::vec3& translation) {
	m_previousState.m_translation = translation;
	m_previousState.m_matNeedsUpdate = true;
	m_currentState.m_translation = translation;
	m_currentState.m_matNeedsUpdate = true;

	
	/*for (auto& ts : m_transformSnapshots) {
		ts.m_translation = translation;
		ts.m_matNeedsUpdate = true;
	}*/
}

void Transform::translate(const float x, const float y, const float z) {
	//m_transformSnapshots[s_updateIndex].m_translation += glm::vec3(x, y, z);
	//m_transformSnapshots[s_updateIndex].m_matNeedsUpdate = true;
	m_currentState.m_translation += glm::vec3(x, y, z);
	m_currentState.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::scale(const float factor) {
	//m_transformSnapshots[s_updateIndex].m_scale *= factor;
	//m_transformSnapshots[s_updateIndex].m_matNeedsUpdate = true;
	m_currentState.m_scale *= factor;
	m_currentState.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::scale(const glm::vec3& scale) {
	//m_transformSnapshots[s_updateIndex].m_scale *= scale;
	//m_transformSnapshots[s_updateIndex].m_matNeedsUpdate = true;
	m_currentState.m_scale *= scale;
	m_currentState.m_matNeedsUpdate = true;
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
	//m_transformSnapshots[s_updateIndex].m_rotation += rotation;
	//m_transformSnapshots[s_updateIndex].m_rotationQuat = glm::quat(m_transformSnapshots[s_updateIndex].m_rotation);
	//m_transformSnapshots[s_updateIndex].m_matNeedsUpdate = true;
	m_currentState.m_rotation += rotation;
	m_currentState.m_rotationQuat = glm::quat(m_currentState.m_rotation);
	m_currentState.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::rotate(const float x, const float y, const float z) {
	//m_transformSnapshots[s_updateIndex].m_rotation += glm::vec3(x, y, z);
	//m_transformSnapshots[s_updateIndex].m_rotationQuat = glm::quat(m_transformSnapshots[s_updateIndex].m_rotation);
	//m_transformSnapshots[s_updateIndex].m_matNeedsUpdate = true;
	m_currentState.m_rotation += glm::vec3(x, y, z);
	m_currentState.m_rotationQuat = glm::quat(m_currentState.m_rotation);
	m_currentState.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::rotateAroundX(const float radians) {
	////////m_transformSnapshots[s_updateIndex].m_rotation.x += radians;
	////////m_transformSnapshots[s_updateIndex].m_rotationQuat = glm::quat(m_transformSnapshots[s_updateIndex].m_rotation);
	////////m_transformSnapshots[s_updateIndex].m_matNeedsUpdate = true;
	m_currentState.m_rotation.x += radians;
	m_currentState.m_rotationQuat = glm::quat(m_currentState.m_rotation);
	m_currentState.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::rotateAroundY(const float radians) {
	//m_transformSnapshots[s_updateIndex].m_rotation.y += radians;
	//m_transformSnapshots[s_updateIndex].m_rotationQuat = glm::quat(m_transformSnapshots[s_updateIndex].m_rotation);
	//m_transformSnapshots[s_updateIndex].m_matNeedsUpdate = true;
	m_currentState.m_rotation.y += radians;
	m_currentState.m_rotationQuat = glm::quat(m_currentState.m_rotation);
	m_currentState.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::rotateAroundZ(const float radians) {
	//m_transformSnapshots[s_updateIndex].m_rotation.z += radians;
	//m_transformSnapshots[s_updateIndex].m_rotationQuat = glm::quat(m_transformSnapshots[s_updateIndex].m_rotation);
	//m_transformSnapshots[s_updateIndex].m_matNeedsUpdate = true;
	m_currentState.m_rotation.z += radians;
	m_currentState.m_rotationQuat = glm::quat(m_currentState.m_rotation);
	m_currentState.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setTranslation(const glm::vec3& translation) {
	//m_transformSnapshots[s_updateIndex].m_translation = translation;
	//m_transformSnapshots[s_updateIndex].m_matNeedsUpdate = true;
	m_currentState.m_translation = translation;
	m_currentState.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setTranslation(const float x, const float y, const float z) {
	m_currentState.m_translation = glm::vec3(x, y, z);
	m_currentState.m_matNeedsUpdate = true;
	m_currentState.m_translation = glm::vec3(x, y, z);
	m_currentState.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setRotations(const glm::vec3& rotations) {
	//m_transformSnapshots[s_updateIndex].m_rotation = rotations;
	//m_transformSnapshots[s_updateIndex].m_rotationQuat = glm::quat(m_transformSnapshots[s_updateIndex].m_rotation);
	//m_transformSnapshots[s_updateIndex].m_matNeedsUpdate = true;
	m_currentState.m_rotation = rotations;
	m_currentState.m_rotationQuat = glm::quat(m_currentState.m_rotation);
	m_currentState.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setRotations(const float x, const float y, const float z) {
	//m_transformSnapshots[s_updateIndex].m_rotation = glm::vec3(x, y, z);
	//m_transformSnapshots[s_updateIndex].m_rotationQuat = glm::quat(m_transformSnapshots[s_updateIndex].m_rotation);
	//m_transformSnapshots[s_updateIndex].m_matNeedsUpdate = true;
	m_currentState.m_rotation = glm::vec3(x, y, z);
	m_currentState.m_rotationQuat = glm::quat(m_currentState.m_rotation);
	m_currentState.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setScale(const float scale) {
	//m_transformSnapshots[s_updateIndex].m_scale = glm::vec3(scale, scale, scale);
	//m_transformSnapshots[s_updateIndex].m_matNeedsUpdate = true;
	m_currentState.m_scale = glm::vec3(scale, scale, scale	);
	m_currentState.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setScale(const float x, const float y, const float z) {
	//m_transformSnapshots[s_updateIndex].m_scale = glm::vec3(x, y, z);
	//m_transformSnapshots[s_updateIndex].m_matNeedsUpdate = true;
	m_currentState.m_scale = glm::vec3(x, y, z	);
	m_currentState.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setScale(const glm::vec3& scale) {
	//m_transformSnapshots[s_updateIndex].m_scale = scale;
	//m_transformSnapshots[s_updateIndex].m_matNeedsUpdate = true;
	m_currentState.m_scale = scale;
	m_currentState.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void Transform::setForward(const glm::vec3& forward) {
	//m_transformSnapshots[s_updateIndex].m_forward = glm::vec3(forward.x, forward.y, -forward.z);
	//m_transformSnapshots[s_updateIndex].m_rotationQuat = glm::rotation(glm::vec3(0.f, 0.f, -1.f), m_transformSnapshots[s_updateIndex].m_forward);
	//m_transformSnapshots[s_updateIndex].m_matNeedsUpdate = true;
	m_currentState.m_forward = glm::vec3(forward.x, forward.y, -forward.z);
	m_currentState.m_rotationQuat = glm::rotation(glm::vec3(0.f, 0.f, -1.f), m_currentState.m_forward);
	m_currentState.m_matNeedsUpdate = true;
}

// NOTE: Not used anywhere at the moment
void Transform::setMatrix(const glm::mat4& newMatrix) {
	m_localTransformMatrix = newMatrix;
	glm::vec3 tempSkew;
	glm::vec4 tempPerspective;
	glm::quat tempRotation;
	glm::decompose(newMatrix, 
		m_currentState.m_scale, 
		tempRotation, 
		m_currentState.m_translation, 
		tempSkew, tempPerspective);
	// TODO: Check that rotation is valid
	m_currentState.m_rotation = glm::eulerAngles(tempRotation);
	m_currentState.m_rotationQuat = glm::quat(m_currentState.m_rotation);

	m_currentState.m_matNeedsUpdate = false;
	treeNeedsUpdating();
}

Transform* Transform::getParent() const {
	return m_parent;
}

// Note: returns the translation/rotation/scale that's currently used in update
const glm::vec3& Transform::getTranslation() const {
	return m_currentState.m_translation;
}
const glm::vec3& Transform::getRotations() const {
	return m_currentState.m_rotation;
}
const glm::vec3& Transform::getScale() const {
	return m_currentState.m_scale;
}

const glm::vec3& Transform::getForward() {
	getMatrix();

	return m_currentState.m_forward;
}

const glm::vec3& Transform::getRight() {
	return m_currentState.m_right;
}

const glm::vec3& Transform::getUp() {
	return m_currentState.m_up;
}

// TODO: rewrite with alpha value
//       Optimize and check for errors.
glm::mat4 Transform::getMatrix(float alpha) {
	// will always need update because of interpolation
	//if (m_currentState.m_matNeedsUpdate) {
	updateLocalMatrix();
	m_currentState.m_matNeedsUpdate = false;
	m_currentState.m_updatedDirections = true;
	//}

	if (m_currentState.m_parentUpdated || !m_parent) {
		updateMatrix();
		m_currentState.m_parentUpdated = false;
		m_currentState.m_updatedDirections = true;
	}
	if (m_currentState.m_updatedDirections) {
		m_currentState.m_up = glm::vec3(glm::vec4(0.f, 1.f, 0.f, 1.f) * m_rotationMatrix);
		//m_up = glm::normalize(m_up);
		m_currentState.m_right = glm::vec3(glm::vec4(1.f, 0.f, 0.f, 1.f) * m_rotationMatrix);
		//m_right = glm::normalize(m_right);
		m_currentState.m_forward = glm::vec3(glm::vec4(0.f, 0.f, 1.f, 1.f) * m_rotationMatrix);
		float x = m_currentState.m_forward.x;
		m_currentState.m_forward.x = m_currentState.m_forward.z;
		m_currentState.m_forward.z = x;
		//m_forward = glm::normalize(m_forward); 
	}

	//if (m_currentState.m_matNeedsUpdate) {
	//	updateLocalMatrix();
	//	m_transformSnapshots[s_renderIndex].m_matNeedsUpdate = false;
	//	m_transformSnapshots[s_renderIndex].m_updatedDirections = true;
	//}
	//if (m_transformSnapshots[s_renderIndex].m_parentUpdated || !m_parent) {
	//	updateMatrix();
	//	m_transformSnapshots[s_renderIndex].m_parentUpdated = false;
	//	m_transformSnapshots[s_renderIndex].m_updatedDirections = true;
	//}
	//if (m_transformSnapshots[s_renderIndex].m_updatedDirections) {
	//	m_transformSnapshots[s_renderIndex].m_up = glm::vec3(glm::vec4(0.f, 1.f, 0.f, 1.f) * m_rotationMatrix);
	//	//m_up = glm::normalize(m_up);
	//	m_transformSnapshots[s_renderIndex].m_right = glm::vec3(glm::vec4(1.f, 0.f, 0.f, 1.f) * m_rotationMatrix);
	//	//m_right = glm::normalize(m_right);
	//	m_transformSnapshots[s_renderIndex].m_forward = glm::vec3(glm::vec4(0.f, 0.f, 1.f, 1.f) * m_rotationMatrix);
	//	float x = m_transformSnapshots[s_renderIndex].m_forward.x;
	//	m_transformSnapshots[s_renderIndex].m_forward.x = m_transformSnapshots[s_renderIndex].m_forward.z;
	//	m_transformSnapshots[s_renderIndex].m_forward.z = x;
	//	//m_forward = glm::normalize(m_forward);
	//}

	return m_transformMatrix;
}

// Not used anywhere
glm::mat4 Transform::getLocalMatrix() {
	/*if (m_transformSnapshots[s_renderIndex].m_matNeedsUpdate) {
		updateLocalMatrix();
		m_transformSnapshots[s_renderIndex].m_matNeedsUpdate = false;
	}*/	
	if (m_currentState.m_matNeedsUpdate) {
		updateLocalMatrix();
		m_currentState.m_matNeedsUpdate = false;
	}
	return m_localTransformMatrix;
}

void Transform::updateLocalMatrix() {
	m_localTransformMatrix = glm::mat4(1.0f);
	glm::mat4 transMatrix = glm::translate(m_localTransformMatrix, m_currentState.m_translation);
	m_rotationMatrix = glm::mat4_cast(m_currentState.m_rotationQuat);
	glm::mat4 scaleMatrix = glm::scale(m_localTransformMatrix, m_currentState.m_scale);
	//m_localTransformMatrix = glm::translate(m_localTransformMatrix, m_translation);
	/*m_localTransformMatrix = glm::rotate(m_localTransformMatrix, m_rotation.x, glm::vec3(1.f, 0.f, 0.f));
	m_localTransformMatrix = glm::rotate(m_localTransformMatrix, m_rotation.y, glm::vec3(0.f, 1.f, 0.f));
	m_localTransformMatrix = glm::rotate(m_localTransformMatrix, m_rotation.z, glm::vec3(0.f, 0.f, 1.f));*/
	//m_localTransformMatrix = glm::scale(m_localTransformMatrix, m_scale);
	m_localTransformMatrix = transMatrix * m_rotationMatrix * scaleMatrix;
}

void Transform::updateMatrix() {
	if (m_parent)
		m_transformMatrix = m_parent->getMatrix() * m_localTransformMatrix;
	else
		m_transformMatrix = m_localTransformMatrix;
}

void Transform::treeNeedsUpdating() {
	m_currentState.m_parentUpdated = true;
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
