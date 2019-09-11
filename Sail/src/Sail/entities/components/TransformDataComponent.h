#pragma once

#include "Component.h"

// TODO: rewrite TransformComponent to make it leaner, use this component for interpolation eventually
// keep the matrixes in it

// TODO: move node to own class with treeneedsupdate function etc.

class TransformDataComponent : public Component {
public:
	SAIL_COMPONENT

	TransformDataComponent(
			const glm::vec3& translation = { 0.0f, 0.0f, 0.0f },
			const glm::vec3& rotation = { 0.0f, 0.0f, 0.0f },
			const glm::vec3& scale = { 1.0f, 1.0f, 1.0f })
		: m_translation(translation),
		m_rotation(rotation),
		m_scale(scale),
		Component()
	{}

	void setTranslation(const glm::vec3& translation) { 
		m_translation = translation; 
		m_dataUpdated = true;
	}

	void setTranslation(const float x, const float y, const float z) {
		m_translation = glm::vec3(x, y, z);
		m_dataUpdated = true;
		treeNeedsUpdating();
	}
	
	void setRotation(const glm::vec3& rotation) { 
		m_rotation = rotation; 
		m_dataUpdated = true;
	}

	void setScale(const float scale) {
		m_scale = glm::vec3(scale, scale, scale);
		m_dataUpdated = true;
		treeNeedsUpdating();
	}

	void setScale(const float x, const float y, const float z) {
		m_scale = glm::vec3(x, y, z);
		m_dataUpdated = true;
		treeNeedsUpdating();
	}

	void setScale(const glm::vec3& scale) { 
		m_scale = scale; 
		m_dataUpdated = true;
	}

	void translate(const glm::vec3& move) {
		m_translation += move;
		m_dataUpdated = true;
		treeNeedsUpdating();
	}

	void translate(const float x, const float y, const float z) {
		m_translation += glm::vec3(x, y, z);
		m_dataUpdated = true;
		treeNeedsUpdating();
	}

	void rotateAroundX(const float radians) {
		m_rotation.x += radians;
		m_dataUpdated = true;
		treeNeedsUpdating();
	}

	void rotateAroundY(const float radians) {
		m_rotation.y += radians;
		m_dataUpdated = true;
		treeNeedsUpdating();
	}

	void rotateAroundZ(const float radians) {
		m_rotation.z += radians;
		m_dataUpdated = true;
		treeNeedsUpdating();
	}



	const glm::vec3& getTranslation() const { return m_translation; }
	const glm::vec3& getRotation() const { return m_rotation; }
	const glm::vec3& getScale() const { return m_scale; }
	const bool wasUpdatedThisTick() const { return m_dataUpdated; }

	// called once the positions have been used to update relevant matrices
	void dataProcessed() { m_dataUpdated = true; }
private:
	// Written to in update loop
	glm::vec3 m_translation;
	glm::vec3 m_rotation;
	glm::vec3 m_scale;

	bool m_dataUpdated;
};