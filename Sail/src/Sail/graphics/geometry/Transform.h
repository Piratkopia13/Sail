#pragma once

#include <glm/glm.hpp>

class Transform {

public:
	Transform() {
		m_scale = glm::vec3(1.0f);
		m_rotation = glm::vec3(0.0f);
		m_translation = glm::vec3(0.f);
		m_transformMatrix = glm::mat4(1.0f);
		m_matNeedsUpdate = false;
		m_parentUpdated = false;
		m_parent = nullptr;
	};
	~Transform() {};

	void setParent(Transform* parent) {
		m_parent = parent;
		m_parentUpdated = true;
		warnChildren();
	}
	void addChild(Transform* child) {
		m_children.push_back(child);
		warnChildren();
	}

	void translate(const glm::vec3& move) {
		m_translation += move;
		m_matNeedsUpdate = true;
		warnChildren();
	}
	void translate(const float x, const float y, const float z) {
		m_translation += glm::vec3(x, y, z);
		m_matNeedsUpdate = true;
		warnChildren();
	}
	void scaleUniformly(float factor) {
		m_scale *= factor;
		m_matNeedsUpdate = true;
		warnChildren();
	}
	void rotateAroundX(float radians) {
		m_rotation.x += radians;
		m_matNeedsUpdate = true;
		warnChildren();
	}
	void rotateAroundY(float radians) {
		m_rotation.y += radians;
		m_matNeedsUpdate = true;
		warnChildren();
	}
	void rotateAroundZ(float radians) {
		m_rotation.z += radians;
		m_matNeedsUpdate = true;
		warnChildren();
	}

	void setTranslation(const glm::vec3& translation) {
		m_translation = translation;
		m_matNeedsUpdate = true;
		warnChildren();
	}
	void setTranslation(const float x, const float y, const float z) {
		m_translation = glm::vec3(x, y, z);
		m_matNeedsUpdate = true;
		warnChildren();
	}
	void setRotations(const glm::vec3& rotations) {
		m_rotation = rotations;
		m_matNeedsUpdate = true;
		warnChildren();
	}
	void setRotations(const float x, const float y, const float z) {
		m_rotation = glm::vec3(x, y, z);
		m_matNeedsUpdate = true;
		warnChildren();
	}
	void setScale(float scale) {
		m_scale = glm::vec3(scale, scale, scale);
		m_matNeedsUpdate = true;
		warnChildren();
	}
	void setNonUniScale(float scalex, float scaley, float scalez) {
		m_scale = glm::vec3(scalex, scaley, scalez);
		m_matNeedsUpdate = true;
		warnChildren();
	}
	void setNonUniScale(const glm::vec3& scale) {
		m_scale = scale;
		m_matNeedsUpdate = true;
		warnChildren();
	}

	void setMatrix(glm::mat4 newMatrix) {
		m_transformMatrix = newMatrix;
		m_matNeedsUpdate = false;
		warnChildren();
	}

	void treeUpdated() {
		m_parentUpdated = true;
		warnChildren();
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
			updateLocalMatrix();
			m_matNeedsUpdate = false;
		}
		if (m_parentUpdated || !m_parent) {
			updateMatrix();
		}
		
		return m_transformMatrix;
	}
	glm::mat4 getLocalMatrix() {
		if (m_matNeedsUpdate) {
			updateLocalMatrix();
			m_matNeedsUpdate = false;
		}
		return m_localTransformMatrix;
	}

private:

	glm::vec3 m_translation;
	glm::vec3 m_rotation;
	glm::vec3 m_scale;

	bool m_matNeedsUpdate;
	bool m_parentUpdated;

	glm::mat4 m_transformMatrix;
	glm::mat4 m_localTransformMatrix;

	Transform* m_parent;
	std::vector<Transform*> m_children;


private:
	void updateLocalMatrix() {
		m_localTransformMatrix = glm::mat4(1.0f);
		m_localTransformMatrix = glm::scale(m_localTransformMatrix, m_scale);
		m_localTransformMatrix = glm::rotate(m_localTransformMatrix, m_rotation.x, glm::vec3(1.f, 0.f, 0.f));
		m_localTransformMatrix = glm::rotate(m_localTransformMatrix, m_rotation.y, glm::vec3(0.f, 1.f, 0.f));
		m_localTransformMatrix = glm::rotate(m_localTransformMatrix, m_rotation.z, glm::vec3(0.f, 0.f, 1.f));
		m_localTransformMatrix = glm::translate(m_localTransformMatrix, m_translation);
	}
	void updateMatrix() {
		if (m_parent)
			m_transformMatrix = m_parent->getMatrix() * m_localTransformMatrix;
		else
			m_transformMatrix = m_localTransformMatrix;
	}
	void warnChildren() {
		for (Transform* child : m_children) {
			child->treeUpdated();
		}
	}
};