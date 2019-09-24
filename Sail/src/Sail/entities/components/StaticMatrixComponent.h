#pragma once
#include "glm/vec3.hpp"
#include "glm/mat4x4.hpp"
#include "Component.h"

// Transform matrix that is set once and never changed
// Use this for any static objects (which neither move, are added, or removed) instead
// of TransformComponent.
//
// TODO: rewrite with quaternions
class StaticMatrixComponent : public Component<StaticMatrixComponent> {
public:
	StaticMatrixComponent(
		const glm::vec3& translation = { 0.0f, 0.0f, 0.0f },
		const glm::vec3& rotation = { 0.0f, 0.0f, 0.0f },
		const glm::vec3& scale = { 1.0f, 1.0f, 1.0f }) 
	{
		//glm::mat4 transMatrix = glm::translate(m_matrix, translation);
		//glm::mat4 rotationMat= glm::mat4_cast(m_currentState.m_rotationQuat);
		//glm::mat4 scaleMatrix = glm::scale(m_localTransformMatrix, m_currentState.m_scale);
		////m_localTransformMatrix = glm::translate(m_localTransformMatrix, m_translation);
		///*m_localTransformMatrix = glm::rotate(m_localTransformMatrix, m_rotation.x, glm::vec3(1.f, 0.f, 0.f));
		//m_localTransformMatrix = glm::rotate(m_localTransformMatrix, m_rotation.y, glm::vec3(0.f, 1.f, 0.f));
		//m_localTransformMatrix = glm::rotate(m_localTransformMatrix, m_rotation.z, glm::vec3(0.f, 0.f, 1.f));*/
		////m_localTransformMatrix = glm::scale(m_localTransformMatrix, m_scale);
		//m_localTransformMatrix = transMatrix * m_rotationMatrix * scaleMatrix;
		m_matrix = glm::translate(m_matrix, translation);
		m_matrix = glm::rotate(m_matrix, rotation.x, glm::vec3(1.f, 0.f, 0.f));
		m_matrix = glm::rotate(m_matrix, rotation.y, glm::vec3(0.f, 1.f, 0.f));
		m_matrix = glm::rotate(m_matrix, rotation.z, glm::vec3(0.f, 0.f, 1.f));
		m_matrix = glm::scale(m_matrix, scale);
	}

	glm::mat4 getMatrix() const { return m_matrix; }

private:
	glm::mat4 m_matrix = glm::mat4(1.0f);
};