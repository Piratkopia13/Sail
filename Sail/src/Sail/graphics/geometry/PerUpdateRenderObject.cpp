#include "pch.h"
#include "PerUpdateRenderObject.h"
#include "Sail/graphics/geometry/Model.h"
#include "Sail/entities/components/TransformComponent.h"
#include "Sail/entities/components/ModelComponent.h"

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

// TODO: rewrite
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


// TODO: rewrite with alpha value
//       Optimize for static objects or make a separate transform type for those
glm::mat4 PerUpdateRenderObject::getMatrix(float alpha) {
	updateLocalMatrix();
	updateMatrix();

	//// will always need update because of interpolation
	////if (m_data.m_current.m_matNeedsUpdate) {
	//updateLocalMatrix();
	////m_data.m_current.m_matNeedsUpdate = false;
	////m_data.m_current.m_updatedDirections = true;
	////}

	//if (m_data.m_current.m_parentUpdated || !m_parent) {
	//	updateMatrix();
	//	m_data.m_current.m_parentUpdated = false;
	//	m_data.m_current.m_updatedDirections = true;
	//}
	//if (m_data.m_current.m_updatedDirections) {
	//	m_data.m_current.m_up = glm::vec3(glm::vec4(0.f, 1.f, 0.f, 1.f) * m_rotationMatrix);
	//	//m_up = glm::normalize(m_up);
	//	m_data.m_current.m_right = glm::vec3(glm::vec4(1.f, 0.f, 0.f, 1.f) * m_rotationMatrix);
	//	//m_right = glm::normalize(m_right);
	//	m_data.m_current.m_forward = glm::vec3(glm::vec4(0.f, 0.f, 1.f, 1.f) * m_rotationMatrix);
	//	float x = m_data.m_current.m_forward.x;
	//	m_data.m_current.m_forward.x = m_data.m_current.m_forward.z;
	//	m_data.m_current.m_forward.z = x;
	//	//m_forward = glm::normalize(m_forward); 
	//}

	////if (m_data.m_current.m_matNeedsUpdate) {
	////	updateLocalMatrix();
	////	m_transformSnapshots[s_renderIndex].m_matNeedsUpdate = false;
	////	m_transformSnapshots[s_renderIndex].m_updatedDirections = true;
	////}
	////if (m_transformSnapshots[s_renderIndex].m_parentUpdated || !m_parent) {
	////	updateMatrix();
	////	m_transformSnapshots[s_renderIndex].m_parentUpdated = false;
	////	m_transformSnapshots[s_renderIndex].m_updatedDirections = true;
	////}
	////if (m_transformSnapshots[s_renderIndex].m_updatedDirections) {
	////	m_transformSnapshots[s_renderIndex].m_up = glm::vec3(glm::vec4(0.f, 1.f, 0.f, 1.f) * m_rotationMatrix);
	////	//m_up = glm::normalize(m_up);
	////	m_transformSnapshots[s_renderIndex].m_right = glm::vec3(glm::vec4(1.f, 0.f, 0.f, 1.f) * m_rotationMatrix);
	////	//m_right = glm::normalize(m_right);
	////	m_transformSnapshots[s_renderIndex].m_forward = glm::vec3(glm::vec4(0.f, 0.f, 1.f, 1.f) * m_rotationMatrix);
	////	float x = m_transformSnapshots[s_renderIndex].m_forward.x;
	////	m_transformSnapshots[s_renderIndex].m_forward.x = m_transformSnapshots[s_renderIndex].m_forward.z;
	////	m_transformSnapshots[s_renderIndex].m_forward.z = x;
	////	//m_forward = glm::normalize(m_forward);
	////}

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
