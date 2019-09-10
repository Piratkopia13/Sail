#pragma once

#include "Component.h"

// TODO: rewrite TransformComponent to make it leaner, use this component for interpolation eventually
// keep the matrixes in it

class StaticPositionComponent : public Component {
	SAIL_COMPONENT
public:
	StaticPositionComponent(
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
	
	void setRotation(const glm::vec3& rotation) { 
		m_rotation = rotation; 
		m_dataUpdated = true;
	}

	void setScale(const glm::vec3& scale) { 
		m_scale = scale; 
		m_dataUpdated = true;
	}

	const glm::vec3& getTranslation() const { return m_translation; }
	const glm::vec3& getRotation() const { return m_rotation; }
	const glm::vec3& getScale() const { return m_scale; }
	const bool getUpdated() const { return m_dataUpdated; }

	// called once the positions have been used to update relevant matrices
	void dataProcessed() { m_dataUpdated = true; }
private:
	// Written to in update loop
	glm::vec3 m_translation;
	glm::vec3 m_rotation;
	glm::vec3 m_scale;

	// Written to in prepare render
	glm::mat4 m_transformMatrix;
	glm::mat4 m_localTransformMatrix;


	bool m_dataUpdated;
};