#pragma once

#include "Camera.h"

class PerspectiveCamera : public Camera {

public:
	PerspectiveCamera(float fov, float aspectRatio, float nearZ, float farZ) 
		: m_fov(glm::radians(fov))
		, m_nearZ(nearZ)
		, m_farZ(farZ)
		, m_aspectRatio(aspectRatio)
	{
		m_projectionMatrix = glm::perspectiveFovLH(m_fov, aspectRatio, 1.0f, m_nearZ, m_farZ);
		/*m_projectionMatrix = glm::scale(m_projectionMatrix, glm::vec3(1.f, 1.f, 0.5f));
		m_projectionMatrix = glm::translate(m_projectionMatrix, glm::vec3(0.f, 0.f, 0.5f));*/

	};

	void resize(int width, int height) {
		m_aspectRatio = static_cast<float>(width) / height;
		m_projectionMatrix = glm::perspectiveFovLH(m_fov, m_aspectRatio, 1.0f, m_nearZ, m_farZ);
		/*m_projectionMatrix = glm::scale(m_projectionMatrix, glm::vec3(1.f, 1.f, 0.5f));
		m_projectionMatrix = glm::translate(m_projectionMatrix, glm::vec3(0.f, 0.f, 0.5f));*/
	}

	float getFOV() const {
		return m_fov;
	}
	float getAspectRatio() const {
		return m_aspectRatio;
	}

private:
	virtual const glm::mat4& getProjectionMatrix() {
		return m_projectionMatrix;
	}

private:
	float m_fov;
	float m_nearZ;
	float m_farZ;
	float m_aspectRatio;

	glm::mat4 m_projectionMatrix;

};
