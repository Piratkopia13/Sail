#pragma once

#include "Component.h"

#include "../../patterns/Node.h"
//#include "TransformDataComponent.h"

class TransformMatrixComponent : public Component {
public:
	SAIL_COMPONENT

	TransformMatrixComponent() : Component() {}
	virtual ~TransformMatrixComponent() {}



	glm::mat4 TransformMatrixComponent::getTransformMatrix() {

		return m_transformMatrix;
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


	void updateMatrix() {
		m_transformMatrix = m_localTransformMatrix;
	}

	void updateMatrixWithParent(const glm::mat4& parentMatrix) {
		m_transformMatrix = parentMatrix * m_localTransformMatrix;
	}

private:
	// Written to in prepare render
	glm::mat4 m_transformMatrix;
	glm::mat4 m_localTransformMatrix;
};