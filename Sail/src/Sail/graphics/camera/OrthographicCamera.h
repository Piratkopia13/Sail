#pragma once

#include "Camera.h"

class OrthographicCamera: public Camera {

public:
	OrthographicCamera(float width, float height, float nearZ, float farZ) {
		nearZDst = nearZ;
		farZDst = farZ;

		m_projectionMatrix = glm::orthoLH(-width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f, nearZ, farZ);
		/*m_projectionMatrix = glm::scale(m_projectionMatrix, glm::vec3(1.f, 1.f, 0.5f));
		m_projectionMatrix = glm::translate(m_projectionMatrix, glm::vec3(0.f, 0.f, 0.5f));*/
	};

	void resize(int width, int height) {
		m_projectionMatrix = glm::orthoLH(-width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f, nearZDst, farZDst);
		/*m_projectionMatrix = glm::scale(m_projectionMatrix, glm::vec3(1.f, 1.f, 0.5f));
		m_projectionMatrix = glm::translate(m_projectionMatrix, glm::vec3(0.f, 0.f, 0.5f));*/
	}

private:
	virtual const glm::mat4& getProjectionMatrix() {
		return m_projectionMatrix;
	}

	glm::mat4 m_projectionMatrix;

};
