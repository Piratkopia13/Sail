#pragma once

#include "Camera.h"

class PerspectiveCamera : public Camera {

public:
	PerspectiveCamera(float fov, float aspectRatio, float nearZ, float farZ) 
		: m_fov(DirectX::XMConvertToRadians(fov))
		, m_nearZ(nearZ)
		, m_farZ(farZ)
		, m_aspectRatio(aspectRatio)
	{
		m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(m_fov, aspectRatio, m_nearZ, m_farZ);
	};

	void resize(int width, int height) {
		m_aspectRatio = static_cast<float>(width) / height;
		m_projectionMatrix = DirectX::XMMatrixPerspectiveFovLH(m_fov, m_aspectRatio, m_nearZ, m_farZ);
	}

	float getFOV() const {
		return m_fov;
	}
	float getAspectRatio() const {
		return m_aspectRatio;
	}

private:
	virtual const DirectX::SimpleMath::Matrix& getProjectionMatrix() {
		return m_projectionMatrix;
	}

private:
	float m_fov;
	float m_nearZ;
	float m_farZ;
	float m_aspectRatio;

	DirectX::SimpleMath::Matrix m_projectionMatrix;

};
