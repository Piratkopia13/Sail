#pragma once

#include <SimpleMath.h>

class Transform {

public:
	Transform() {
		m_scale = DirectX::SimpleMath::Vector3(1.0f, 1.0f, 1.0f);
		m_transformMatrix = DirectX::SimpleMath::Matrix::Identity;
		m_matNeedsUpdate = false;
	};
	~Transform() {};

	void translate(const DirectX::SimpleMath::Vector3& move) {
		m_translation += move;
		m_matNeedsUpdate = true;
	}
	void scaleUniformly(float factor) {
		m_scale *= factor;
		m_matNeedsUpdate = true;
	}
	void rotateAroundX(float radians) {
		m_rotation.x += radians;
		m_matNeedsUpdate = true;
	}
	void rotateAroundY(float radians) {
		m_rotation.y += radians;
		m_matNeedsUpdate = true;
	}
	void rotateAroundZ(float radians) {
		m_rotation.z += radians;
		m_matNeedsUpdate = true;
	}

	void setTranslation(const DirectX::SimpleMath::Vector3& translation) {
		m_translation = translation;
		m_matNeedsUpdate = true;
	}
	void setRotations(const DirectX::SimpleMath::Vector3& rotations) {
		m_rotation = rotations;
		m_matNeedsUpdate = true;
	}
	void setScale(float scale) {
		m_scale = DirectX::SimpleMath::Vector3(scale, scale, scale);
		m_matNeedsUpdate = true;
	}


	void setNonUniScale(float scalex, float scaley, float scalez) {
		m_scale = DirectX::SimpleMath::Vector3(scalex, scaley, scalez);
		m_matNeedsUpdate = true;
	}

	void setMatrix(DirectX::SimpleMath::Matrix newMatrix) {
		m_transformMatrix = newMatrix;
		m_matNeedsUpdate = false;
	}

	const DirectX::SimpleMath::Vector3& getTranslation() const {
		return m_translation;
	}
	const DirectX::SimpleMath::Vector3& getRotations() const {
		return m_rotation;
	}
	const DirectX::SimpleMath::Vector3 getScale() const {
		return m_scale;
	}


	DirectX::SimpleMath::Matrix getMatrix() {
		if (m_matNeedsUpdate) {
			m_transformMatrix = DirectX::SimpleMath::Matrix::CreateScale(m_scale)
				* DirectX::SimpleMath::Matrix::CreateRotationX(m_rotation.x)
				* DirectX::SimpleMath::Matrix::CreateRotationY(m_rotation.y)
				* DirectX::SimpleMath::Matrix::CreateRotationZ(m_rotation.z)
				* DirectX::SimpleMath::Matrix::CreateTranslation(m_translation);

				m_matNeedsUpdate = false;
		}
		return m_transformMatrix;
	}

private:

	DirectX::SimpleMath::Vector3 m_translation;
	DirectX::SimpleMath::Vector3 m_rotation;
	DirectX::SimpleMath::Vector3 m_scale;

	bool m_matNeedsUpdate;
	DirectX::SimpleMath::Matrix m_transformMatrix;

};