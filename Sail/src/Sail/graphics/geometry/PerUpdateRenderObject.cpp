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

PerUpdateRenderObject::~PerUpdateRenderObject() 
{}

void PerUpdateRenderObject::setParent(PerUpdateRenderObject* parent) {
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



// NOTE: Not used anywhere at the moment
void PerUpdateRenderObject::setMatrix(const glm::mat4& newMatrix) {
	m_localTransformMatrix = newMatrix;
	glm::vec3 tempSkew;
	glm::vec4 tempPerspective;
	glm::quat tempRotation;
	glm::decompose(newMatrix, 
		m_data.m_current.m_scale, 
		tempRotation, 
		m_data.m_current.m_translation, 
		tempSkew, tempPerspective);
	// TODO: Check that rotation is valid
	m_data.m_current.m_rotation = glm::eulerAngles(tempRotation);
	m_data.m_current.m_rotationQuat = glm::quat(m_data.m_current.m_rotation);

	//m_data.m_current.m_matNeedsUpdate = false;
	treeNeedsUpdating();
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
	//m_localTransformMatrix = glm::translate(m_localTransformMatrix, m_translation);
	/*m_localTransformMatrix = glm::rotate(m_localTransformMatrix, m_rotation.x, glm::vec3(1.f, 0.f, 0.f));
	m_localTransformMatrix = glm::rotate(m_localTransformMatrix, m_rotation.y, glm::vec3(0.f, 1.f, 0.f));
	m_localTransformMatrix = glm::rotate(m_localTransformMatrix, m_rotation.z, glm::vec3(0.f, 0.f, 1.f));*/
	//m_localTransformMatrix = glm::scale(m_localTransformMatrix, m_scale);
	m_localTransformMatrix = transMatrix * m_rotationMatrix * scaleMatrix;
	m_transformMatrix = m_localTransformMatrix;


	return m_transformMatrix;
}

// Not used anywhere
//glm::mat4 RenderTransform::getLocalMatrix() {
//	/*if (m_transformSnapshots[s_renderIndex].m_matNeedsUpdate) {
//	updateLocalMatrix();
//	m_transformSnapshots[s_renderIndex].m_matNeedsUpdate = false;
//	}*/	
//	if (m_data.m_current.m_matNeedsUpdate) {
//		updateLocalMatrix();
//		m_data.m_current.m_matNeedsUpdate = false;
//	}
//	return m_localTransformMatrix;
//}

void PerUpdateRenderObject::updateLocalMatrix() {
	m_localTransformMatrix = glm::mat4(1.0f);
	glm::mat4 transMatrix = glm::translate(m_localTransformMatrix, m_data.m_current.m_translation);
	m_rotationMatrix = glm::mat4_cast(m_data.m_current.m_rotationQuat);
	glm::mat4 scaleMatrix = glm::scale(m_localTransformMatrix, m_data.m_current.m_scale);
	//m_localTransformMatrix = glm::translate(m_localTransformMatrix, m_translation);
	/*m_localTransformMatrix = glm::rotate(m_localTransformMatrix, m_rotation.x, glm::vec3(1.f, 0.f, 0.f));
	m_localTransformMatrix = glm::rotate(m_localTransformMatrix, m_rotation.y, glm::vec3(0.f, 1.f, 0.f));
	m_localTransformMatrix = glm::rotate(m_localTransformMatrix, m_rotation.z, glm::vec3(0.f, 0.f, 1.f));*/
	//m_localTransformMatrix = glm::scale(m_localTransformMatrix, m_scale);
	m_localTransformMatrix = transMatrix * m_rotationMatrix * scaleMatrix;
}

void PerUpdateRenderObject::updateMatrix() {
	if (m_parent)
		m_transformMatrix = m_parent->getMatrix() * m_localTransformMatrix;
	else
		m_transformMatrix = m_localTransformMatrix;
}

void PerUpdateRenderObject::treeNeedsUpdating() {
	//m_data.m_current.m_parentUpdated = true;
	for (PerUpdateRenderObject* child : m_children) {
		child->treeNeedsUpdating();
	}
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
