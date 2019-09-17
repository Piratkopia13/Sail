#include "pch.h"
#include "GameTransform.h"
#include "RenderTransform.h"

GameTransform::GameTransform(GameTransform* parent)
	: GameTransform::GameTransform({ 0.0f, 0.0f, 0.0f }, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, parent) 
{}

//GameTransform::GameTransform(TransformSnapshot current, TransformSnapshot prev) 
//	: m_currentState(current), m_previousState(prev) 
//{}


GameTransform::GameTransform(const glm::vec3& translation, GameTransform* parent)
	: GameTransform(translation, { 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f, 1.0f }, parent) 
{}

GameTransform::GameTransform(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale, GameTransform* parent)
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

GameTransform::~GameTransform() {}

void GameTransform::setParent(GameTransform* parent) {
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

void GameTransform::removeParent() {
	if (m_parent) {
		m_parent->removeChild(this);
		m_parent = nullptr;
	}
}

// NOTE: Has to be done at the beginning of each update
// copies current state into previous state
void GameTransform::prepareUpdate() {
	m_data.m_previous = m_data.m_previous;
	//m_transformSnapshots[s_updateIndex] = m_transformSnapshots[prevInd(s_updateIndex)];
}

// NOT USED
TransformSnapshot GameTransform::getCurrentTransformState() const {
	return m_data.m_current;
}

TransformSnapshot GameTransform::getPreviousTransformState() const {
	return m_data.m_previous;
}


TransformFrame GameTransform::getTransformFrame() const {
	return m_data;
}

// TODO: Rewrite with RenderTransform
// Returns a copy with the current and previous snapshots as well as copies of the parents' snapshots
//GameTransform* GameTransform::getTransformSnapshot() const {
//	GameTransform* toReturn = SAIL_NEW GameTransform(m_currentState, m_previousState);
//	if (m_parent) {
//		toReturn->setParent(m_parent->getTransformSnapshot());
//	}
//	return toReturn;
//}

//void GameTransform::copySnapshotFromObject(GameTransform* object) {
//	m_previousState= object->getPreviousTransformState();
//	m_currentState = object->getCurrentTransformState();
//
//	// If the object has a parent GameTransform copy that one as well,
//	// NOTE: this will cause duplication of some GameTransforms
//	if (GameTransform* parent = object->getParent(); parent) {
//		//setParent(parent->getTransformSnapshot());
//	}
//}


void GameTransform::translate(const glm::vec3& move) {
	//m_transformSnapshots[s_updateIndex].m_translation += move;
	//m_transformSnapshots[s_updateIndex].m_matNeedsUpdate = true;
	m_data.m_current.m_translation += move;
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void GameTransform::setStartTranslation(const glm::vec3& translation) {
	m_data.m_previous.m_translation = translation;
	//m_data.m_previous.m_matNeedsUpdate = true;
	m_data.m_current.m_translation = translation;
	//m_data.m_current.m_matNeedsUpdate = true;


	/*for (auto& ts : m_transformSnapshots) {
	ts.m_translation = translation;
	ts.m_matNeedsUpdate = true;
	}*/
}

void GameTransform::translate(const float x, const float y, const float z) {
	//m_transformSnapshots[s_updateIndex].m_translation += glm::vec3(x, y, z);
	//m_transformSnapshots[s_updateIndex].m_matNeedsUpdate = true;
	m_data.m_current.m_translation += glm::vec3(x, y, z);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void GameTransform::scale(const float factor) {
	//m_transformSnapshots[s_updateIndex].m_scale *= factor;
	//m_transformSnapshots[s_updateIndex].m_matNeedsUpdate = true;
	m_data.m_current.m_scale *= factor;
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void GameTransform::scale(const glm::vec3& scale) {
	//m_transformSnapshots[s_updateIndex].m_scale *= scale;
	//m_transformSnapshots[s_updateIndex].m_matNeedsUpdate = true;
	m_data.m_current.m_scale *= scale;
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

/*void GameTransform::rotate(const glm::vec3& rotation) {
m_rotation += rotation;
m_matNeedsUpdate = true;
treeNeedsUpdating();
}

void GameTransform::rotate(const float x, const float y, const float z) {
m_rotation += glm::vec3(x, y, z);
m_matNeedsUpdate = true;
treeNeedsUpdating();
}*/
void GameTransform::rotate(const glm::vec3& rotation) {
	m_data.m_current.m_rotation += rotation;
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void GameTransform::rotate(const float x, const float y, const float z) {
	m_data.m_current.m_rotation += glm::vec3(x, y, z);
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void GameTransform::rotateAroundX(const float radians) {
	m_data.m_current.m_rotation.x += radians;
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void GameTransform::rotateAroundY(const float radians) {
	m_data.m_current.m_rotation.y += radians;
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void GameTransform::rotateAroundZ(const float radians) {
	m_data.m_current.m_rotation.z += radians;
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void GameTransform::setTranslation(const glm::vec3& translation) {
	m_data.m_current.m_translation = translation;
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void GameTransform::setTranslation(const float x, const float y, const float z) {
	m_data.m_current.m_translation = glm::vec3(x, y, z);
	//m_data.m_current.m_matNeedsUpdate = true;
	m_data.m_current.m_translation = glm::vec3(x, y, z);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void GameTransform::setRotations(const glm::vec3& rotations) {
	m_data.m_current.m_rotation = rotations;
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void GameTransform::setRotations(const float x, const float y, const float z) {
	m_data.m_current.m_rotation = glm::vec3(x, y, z);
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void GameTransform::setScale(const float scale) {
	m_data.m_current.m_scale = glm::vec3(scale, scale, scale	);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void GameTransform::setScale(const float x, const float y, const float z) {
	m_data.m_current.m_scale = glm::vec3(x, y, z	);
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void GameTransform::setScale(const glm::vec3& scale) {
	m_data.m_current.m_scale = scale;
	//m_data.m_current.m_matNeedsUpdate = true;
	treeNeedsUpdating();
}

void GameTransform::setForward(const glm::vec3& forward) {
	m_data.m_current.m_forward = glm::vec3(forward.x, forward.y, -forward.z);
	m_data.m_current.m_rotationQuat = glm::rotation(glm::vec3(0.f, 0.f, -1.f), m_data.m_current.m_forward);
	//m_data.m_current.m_matNeedsUpdate = true;
}

//// NOTE: Not used anywhere at the moment
//void GameTransform::setMatrix(const glm::mat4& newMatrix) {
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

GameTransform* GameTransform::getParent() const {
	return m_parent;
}


// creates a new RenderTransform
RenderTransform* GameTransform::getRenderTransform() const {
	RenderTransform* toReturn = SAIL_NEW RenderTransform();
	if (m_parent) {
		toReturn->setParent(m_parent->getRenderTransform());
	}
	return toReturn;
}


// Note: returns the translation/rotation/scale that's currently used in update
const glm::vec3& GameTransform::getTranslation() const {
	return m_data.m_current.m_translation;
}
const glm::vec3& GameTransform::getRotations() const {
	return m_data.m_current.m_rotation;
}
const glm::vec3& GameTransform::getScale() const {
	return m_data.m_current.m_scale;
}
//
//const glm::vec3& GameTransform::getForward() {
//	getMatrix();
//
//	return m_data.m_current.m_forward;
//}

//const glm::vec3& GameTransform::getRight() {
//	return m_data.m_current.m_right;
//}
//
//const glm::vec3& GameTransform::getUp() {
//	return m_data.m_current.m_up;
//}



void GameTransform::treeNeedsUpdating() {
	//m_data.m_current.m_parentUpdated = true;
	//for (GameTransform* child : m_children) {
	//	child->treeNeedsUpdating();
	//}
}

void GameTransform::addChild(GameTransform* transform) {
	m_children.push_back(transform);
}

void GameTransform::removeChild(GameTransform* GameTransform) {
	for (int i = 0; i < m_children.size(); i++) {
		if (m_children[i] == GameTransform) {
			m_children[i] = m_children.back();
			m_children.pop_back();
			break;
		}
	}
}
