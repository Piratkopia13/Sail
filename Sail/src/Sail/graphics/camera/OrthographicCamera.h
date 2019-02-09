#pragma once

#include "Camera.h"

class OrthographicCamera: public Camera {

public:
	OrthographicCamera(float width, float height, float nearZ, float farZ)
		: m_nearZ(nearZ)
		, m_farZ(farZ)
	{
		m_projectionMatrix = DirectX::XMMatrixOrthographicLH(width, height, m_nearZ, m_farZ);
	};

	void resize(int width, int height) {
		m_projectionMatrix = DirectX::XMMatrixOrthographicLH(static_cast<float>(width), static_cast<float>(height), m_nearZ, m_farZ);
	}

private:
	virtual const DirectX::SimpleMath::Matrix& getProjectionMatrix() {
		return m_projectionMatrix;
	}

	float m_nearZ;
	float m_farZ;

	DirectX::SimpleMath::Matrix m_projectionMatrix;

};
