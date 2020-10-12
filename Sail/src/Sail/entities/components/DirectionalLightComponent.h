#pragma once

#include "Component.h"
#include <glm/glm.hpp>

class DirectionalLightComponent : public Component {
public:
	DirectionalLightComponent(const glm::vec3& color = glm::vec3(.8f), const glm::vec3& dir = glm::vec3(0.f, -1.f, 0.f));
	DirectionalLightComponent(const DirectionalLightComponent&) = default;

	const glm::vec3& getColor() const;
	void setColor(const glm::vec3& color);
	const glm::vec3& getDirection() const;
	void setDirection(const glm::vec3& dir);
	void setIntensity(float intensity);
	float getIntensity();

	virtual void renderEditorGui(SailGuiWindow* window) override;

private:
	glm::vec3 m_color;
	glm::vec3 m_direction;
	float m_intensity;
};
