#pragma once

#include <d3d11.h>
#include <SimpleMath.h>
#include "../geometry/spatial/AABB.h"

struct Frustum {
	// In order of left, right, bottom, top, near, far
	DirectX::SimpleMath::Vector4 planes[6];

	bool containsOrIntersects(const AABB& aabb) const { 

		for (int i = 0; i < 6; i++) {
			if (!planeAABBIntersects(planes[i], aabb))
				return false;
		}

		return true; 

	}
	bool planeAABBIntersects(const DirectX::SimpleMath::Vector4& plane, const AABB& aabb) const {

		DirectX::SimpleMath::Vector4 c(aabb.getCenterPos());
		c.w = 1.f;
		DirectX::SimpleMath::Vector3 h = aabb.getHalfSizes();
		float e = h.x*fabs(plane.x) + h.y*fabs(plane.y) + h.z*fabs(plane.z);
		float s = c.Dot(plane);

		if (s-e > 0)
			return false; // Outside

		// Else inside or intersecting
		return true;
	}
	void extractPlanes(const DirectX::SimpleMath::Matrix& vp) {

		planes[0].x = -(vp._14 + vp._11);
		planes[0].y = -(vp._24 + vp._21);
		planes[0].z = -(vp._34 + vp._31);
		planes[0].w = -(vp._44 + vp._41);

		planes[1].x = -(vp._14 - vp._11);
		planes[1].y = -(vp._24 - vp._21);
		planes[1].z = -(vp._34 - vp._31);
		planes[1].w = -(vp._44 - vp._41);

		planes[2].x = -(vp._14 + vp._12);
		planes[2].y = -(vp._24 + vp._22);
		planes[2].z = -(vp._34 + vp._32);
		planes[2].w = -(vp._44 + vp._42);

		planes[3].x = -(vp._14 - vp._12);
		planes[3].y = -(vp._24 - vp._22);
		planes[3].z = -(vp._34 - vp._32);
		planes[3].w = -(vp._44 - vp._42);

		planes[4].x = -(vp._13);
		planes[4].y = -(vp._23);
		planes[4].z = -(vp._33);
		planes[4].w = -(vp._43);

		planes[5].x = -(vp._14 - vp._13);
		planes[5].y = -(vp._24 - vp._23);
		planes[5].z = -(vp._34 - vp._33);
		planes[5].w = -(vp._44 - vp._43);
	}
};

class Camera {
	friend class CameraController;
public:

	Camera() {
		m_viewMatrix = DirectX::SimpleMath::Matrix::Identity;

		m_pos = DirectX::SimpleMath::Vector3(0.f, 0.f, 0.f);
		m_direction = DirectX::SimpleMath::Vector3(0.f, 0.f, -1.f);
		m_up = DirectX::SimpleMath::Vector3::Up;

		m_updateVPMatrix = true;
		m_updateViewMatrix = true;
		m_updateUp = true;
	}

	void lookAt(const DirectX::SimpleMath::Vector3& pos) {
		m_direction = pos - m_pos;
		m_direction.Normalize();
		m_updateViewMatrix = true;
	}
	void setPosition(const DirectX::SimpleMath::Vector3& pos) {
		m_pos = pos;
		m_updateViewMatrix = true;
	}
	const DirectX::SimpleMath::Vector3& getPosition() const {
		return m_pos;
	}
	void setDirection(const DirectX::SimpleMath::Vector3& dir) {
		m_direction = dir;
		m_updateViewMatrix = true;
		m_updateUp = true;
	}
	const DirectX::SimpleMath::Vector3& getDirection() const {
		return m_direction;
	}
	

	const DirectX::SimpleMath::Matrix& getViewProjection() {

		if (m_updateViewMatrix) {
			m_viewMatrix = DirectX::XMMatrixLookAtLH(m_pos, m_pos + m_direction, DirectX::SimpleMath::Vector3::Up);
			m_updateVPMatrix = true;
			m_updateViewMatrix = false;
		}

		if (m_updateVPMatrix) {
			m_VPMatrix = m_viewMatrix * getProjectionMatrix();
			// Update frustum planes
			m_frustum.extractPlanes(m_VPMatrix);
			m_updateVPMatrix = false;
		}

		return m_VPMatrix;
	}

	const DirectX::SimpleMath::Matrix& getViewMatrix() {
		if (m_updateViewMatrix) {
			m_viewMatrix = DirectX::XMMatrixLookAtLH(m_pos, m_pos + m_direction, DirectX::SimpleMath::Vector3::Up);
			m_updateVPMatrix = true;
			m_updateViewMatrix = false;
		}
		return m_viewMatrix;
	}
	const DirectX::SimpleMath::Matrix& getProjMatrix() {
		return getProjectionMatrix();
	}

	const DirectX::SimpleMath::Vector3& getUp() {
		if (m_updateUp) {
			DirectX::SimpleMath::Vector3 right = DirectX::SimpleMath::Vector3::Up.Cross(m_direction);
			m_up = m_direction.Cross(right);
			m_up.Normalize();
			m_updateUp = false;
		}
		return m_up;
	}

	const Frustum& getFrustum() {
		// Update frustum planes to the latest if VP matrix needs to be updated
		if (m_updateViewMatrix || m_updateVPMatrix)
			m_frustum.extractPlanes(getViewProjection());
		return m_frustum;
	}

protected:
	DirectX::SimpleMath::Matrix m_VPMatrix;

	DirectX::SimpleMath::Matrix m_viewMatrix;

private:
	virtual const DirectX::SimpleMath::Matrix& getProjectionMatrix() = 0;

private:
	bool m_updateVPMatrix, m_updateViewMatrix;
	bool m_updateUp;

	DirectX::SimpleMath::Vector3 m_pos;
	DirectX::SimpleMath::Vector3 m_direction;
	DirectX::SimpleMath::Vector3 m_up;

	Frustum m_frustum;

};
