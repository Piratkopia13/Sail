#pragma once

#include "Component.h"
#include <glm/glm.hpp>

class PointLightComponent : public Component {
public:
	PointLightComponent();
	PointLightComponent(const PointLightComponent&) = default;

	void setColor(const glm::vec3& color);
	const glm::vec3& getColor() const;
	void setPosition(const glm::vec3& position);
	const glm::vec3& getPosition() const;
	void setAttenuationRadius(float radius);
	float getAttenuationRadius() const;
	void setIntensity(float intensity);
	float getIntensity();

	virtual void renderEditorGui(SailGuiWindow* window) override;

private:
	glm::vec3 m_color;
	glm::vec3 m_position;
	float m_attenuationRadius;
	float m_intensity;
};
