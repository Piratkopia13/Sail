#pragma once

#include <glm/glm.hpp>

class Transform {

public:
	Transform() {
		m_scale = glm::vec3(1.0f, 1.0f, 1.0f);
		m_transformMatrix = glm::mat4(1.0f);
		m_matNeedsUpdate = false;
	};
	~Transform() {};

	void translate(const glm::vec3& move) {
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

	void setTranslation(const glm::vec3& translation) {
		m_translation = translation;
		m_matNeedsUpdate = true;
	}
	void setRotations(const glm::vec3& rotations) {
		m_rotation = rotations;
		m_matNeedsUpdate = true;
	}
	void setScale(float scale) {
		m_scale = glm::vec3(scale, scale, scale);
		m_matNeedsUpdate = true;
	}


	void setNonUniScale(float scalex, float scaley, float scalez) {
		m_scale = glm::vec3(scalex, scaley, scalez);
		m_matNeedsUpdate = true;
	}

	void setMatrix(glm::mat4 newMatrix) {
		m_transformMatrix = newMatrix;
		m_matNeedsUpdate = false;
	}

	const glm::vec3& getTranslation() const {
		return m_translation;
	}
	const glm::vec3& getRotations() const {
		return m_rotation;
	}
	const glm::vec3 getScale() const {
		return m_scale;
	}


	glm::mat4 getMatrix() {
		if (m_matNeedsUpdate) {
			m_transformMatrix = glm::scale(m_transformMatrix, m_scale);
			m_transformMatrix = glm::rotate(m_transformMatrix, m_rotation.x, glm::vec3(1.f, 0.f, 0.f));
			m_transformMatrix = glm::rotate(m_transformMatrix, m_rotation.y, glm::vec3(0.f, 1.f, 0.f));
			m_transformMatrix = glm::rotate(m_transformMatrix, m_rotation.z, glm::vec3(0.f, 0.f, 1.f));
			m_transformMatrix = glm::translate(m_transformMatrix, m_translation);

			m_matNeedsUpdate = false;
		}
		return m_transformMatrix;
	}

private:

	glm::vec3 m_translation;
	glm::vec3 m_rotation;
	glm::vec3 m_scale;

	bool m_matNeedsUpdate;
	glm::mat4 m_transformMatrix;

};