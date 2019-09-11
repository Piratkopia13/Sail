#pragma once

#include "Component.h"

#include "../../patterns/Node.h"

class TransformMatrixComponent : public Component, public Node<TransformMatrixComponent> {
public:
	SAIL_COMPONENT

	TransformMatrixComponent() : Component(), Node(this) {}
	TransformMatrixComponent(TransformMatrixComponent* parent) : Node(this, parent) {}

	virtual ~TransformMatrixComponent() {}


	//void TransformMatrixComponent::setParent(TransformMatrixComponent* parent) {
	//	if (m_parent) {
	//		m_parent->removeChild(this);
	//	}
	//	m_parent = parent;
	//	parent->addChild(this);
	//	m_parentUpdated = true;
	//	treeNeedsUpdating();
	//}

	//void TransformMatrixComponent::removeParent() {
	//	if (m_parent) {
	//		m_parent->removeChild(this);
	//		m_parent = nullptr;
	//	}
	//}

	//void TransformMatrixComponent::addChild(TransformMatrixComponent* transform) {
	//	m_children.push_back(transform);
	//}

	//void TransformMatrixComponent::removeChild(TransformMatrixComponent* transform) {
	//	for (int i = 0; i < m_children.size(); i++) {
	//		if (m_children[i] == transform) {
	//			m_children[i] = m_children.back();
	//			m_children.pop_back();
	//			break;
	//		}
	//	}
	//}

	//void TransformMatrixComponent::treeNeedsUpdating() {
	//	m_parentUpdated = true;
	//	for (TransformMatrixComponent* child : m_children) {
	//		child->treeNeedsUpdating();
	//	}
	//}

	//bool TransformMatrixComponent::hasParent() const {
	//	return m_parent;
	//}

	//bool TransformMatrixComponent::getParentUpdated() const {
	//	return m_parentUpdated;
	//}

	//void TransformMatrixComponent::setParentUpdated(bool updated) {
	//	m_parentUpdated = updated;
	//}

	glm::mat4 TransformMatrixComponent::getMatrix() {
		return m_transformMatrix;
	}

	glm::mat4 TransformMatrixComponent::getLocalMatrix() {
		return m_localTransformMatrix;
	}

	void TransformMatrixComponent::updateLocalMatrix(const glm::vec3& translation, const glm::vec3& rotation, const glm::vec3& scale) {
		m_localTransformMatrix = glm::mat4(1.0f);
		m_localTransformMatrix = glm::translate(m_localTransformMatrix, translation);
		m_localTransformMatrix = glm::rotate(m_localTransformMatrix, rotation.x, glm::vec3(1.f, 0.f, 0.f));
		m_localTransformMatrix = glm::rotate(m_localTransformMatrix, rotation.y, glm::vec3(0.f, 1.f, 0.f));
		m_localTransformMatrix = glm::rotate(m_localTransformMatrix, rotation.z, glm::vec3(0.f, 0.f, 1.f));
		m_localTransformMatrix = glm::scale(m_localTransformMatrix, scale);

	}

	void TransformMatrixComponent::setLocalMatrix(const glm::mat4& newMatrix) {
		m_localTransformMatrix = newMatrix;
	}


	void TransformMatrixComponent::updateMatrix() {
		if (hasParent()) {
			m_transformMatrix = getParent()->getDataPtr()->getMatrix() * m_localTransformMatrix;
		} else {
			m_transformMatrix = m_localTransformMatrix;
		}
		setParentUpdated(false);
	}

private:
	// Written to in prepare render
	glm::mat4 m_transformMatrix;
	glm::mat4 m_localTransformMatrix;

	//bool m_parentUpdated = false;

	//TransformMatrixComponent* m_parent = nullptr;
	//std::vector<TransformMatrixComponent*> m_children;

	Node<glm::mat4> test;
};