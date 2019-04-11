#pragma once

#include "Camera.h"

class OrthographicCamera: public Camera {

public:
	OrthographicCamera(float width, float height, float nearZ, float farZ)
		: m_nearZ(nearZ)
		, m_farZ(farZ)
	{
		m_projectionMatrix = glm::orthoLH(-width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f, m_nearZ, m_farZ);
	};

	void resize(int width, int height) {
		m_projectionMatrix = glm::orthoLH(-width / 2.0f, width / 2.0f, -height / 2.0f, height / 2.0f, m_nearZ, m_farZ);
	}

private:
	virtual const glm::mat4& getProjectionMatrix() {
		return m_projectionMatrix;
	}

	float m_nearZ;
	float m_farZ;

	glm::mat4 m_projectionMatrix;

};
