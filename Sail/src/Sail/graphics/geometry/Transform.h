#pragma once

#include <glm/glm.hpp>
#include <glm/gtx/matrix_decompose.hpp>

class Transform {

public:
	Transform(Transform* parent)
		: Transform::Transform() {
		m_parent = parent;
		m_parentUpdated = true;
		if (m_parent)
			m_parent->addChild(this);
	}
	Transform(const glm::vec3& translation, Transform* parent = nullptr)
		: Transform() {
		m_translation = translation;
		m_parent = parent;
		m_parentUpdated = parent;
		if (m_parent)
			m_parent->addChild(this);
	}
	Transform(const glm::vec3& translation = { 0.0f, 0.0f, 0.0f }, const glm::vec3& rotation = { 0.0f, 0.0f, 0.0f }, const glm::vec3& scale = { 1.0f, 1.0f, 1.0f }, Transform* parent = nullptr)
		: m_translation(translation)
		, m_rotation(rotation)
		, m_scale(scale)
		, m_transformMatrix(1.0f)
		, m_matNeedsUpdate(true)
		, m_parentUpdated(parent)
		, m_parent(parent)
	{ 
		if(m_parent)
			m_parent->addChild(this);
	}
	virtual ~Transform() {}

	void setParent(Transform* parent) {
		if (m_parent) {
			m_parent->removeChild(this);
		}
		m_parent = parent;
		parent->addChild(this);
		m_parentUpdated = true;
		warnChildren();
	}
	void removeParent() {
		if (m_parent) {
			m_parent->removeChild(this);
			m_parent = nullptr;
		}
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
	
	void scale(float factor) {
		m_scale *= factor;
		m_matNeedsUpdate = true;
		warnChildren();
	}
	void scale(const glm::vec3& scale) {
		m_scale *= scale;
		m_matNeedsUpdate = true;
		warnChildren();
	}
	
	void rotate(const glm::vec3& rotation) {
		m_rotation += rotation;
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
	void setScale(float scalex, float scaley, float scalez) {
		m_scale = glm::vec3(scalex, scaley, scalez);
		m_matNeedsUpdate = true;
		warnChildren();
	}
	void setScale(const glm::vec3& scale) {
		m_scale = scale;
		m_matNeedsUpdate = true;
		warnChildren();
	}

	void setMatrix(const glm::mat4& newMatrix) {
		m_localTransformMatrix = newMatrix;
		glm::vec3 tempSkew;
		glm::vec4 tempPerspective;
		glm::quat tempRotation;
		glm::decompose(newMatrix, m_scale, tempRotation, m_translation, tempSkew, tempPerspective);
		// TODO: convert from quaternion into euler angles
			   

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
	const glm::vec3& getScale() const {
		return m_scale;
	}


	glm::mat4 getMatrix() {
		if (m_matNeedsUpdate) {
			updateLocalMatrix();
			m_matNeedsUpdate = false;
		}
		if (m_parentUpdated || !m_parent) {
			updateMatrix();
			m_parentUpdated = false;
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
		m_localTransformMatrix = glm::translate(m_localTransformMatrix, m_translation);
		m_localTransformMatrix = glm::rotate(m_localTransformMatrix, m_rotation.x, glm::vec3(1.f, 0.f, 0.f));
		m_localTransformMatrix = glm::rotate(m_localTransformMatrix, m_rotation.y, glm::vec3(0.f, 1.f, 0.f));
		m_localTransformMatrix = glm::rotate(m_localTransformMatrix, m_rotation.z, glm::vec3(0.f, 0.f, 1.f));
		m_localTransformMatrix = glm::scale(m_localTransformMatrix, m_scale);
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
	void addChild(Transform* transform) {
		m_children.push_back(transform);
	}
	void removeChild(Transform* transform) {
		for(int i = 0; i < m_children.size(); i++) {
			if (m_children[i] == transform) {
				m_children[i] = m_children.back();
				m_children.pop_back();
				break;
			}
		}
	}
};