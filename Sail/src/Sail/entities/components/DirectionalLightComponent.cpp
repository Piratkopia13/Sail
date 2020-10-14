#include "pch.h"
#include "DirectionalLightComponent.h"

#include "Sail/gui/SailGuiWindow.h"
#include "imgui.h"

DirectionalLightComponent::DirectionalLightComponent(const glm::vec3& color, const glm::vec3& dir)
	: m_color(color)
	, m_direction(dir)
	, m_intensity(1.f)
{ }

const glm::vec3& DirectionalLightComponent::getColor() const {
	return m_color;
}

void DirectionalLightComponent::setColor(const glm::vec3& color) {
	m_color = color;
}

const glm::vec3& DirectionalLightComponent::getDirection() const {
	return m_direction;
}

void DirectionalLightComponent::setDirection(const glm::vec3& dir) {
	m_direction = dir;
}

void DirectionalLightComponent::setIntensity(float intensity) {
	m_intensity = intensity;
}

float DirectionalLightComponent::getIntensity() {
	return m_intensity;
}

void DirectionalLightComponent::renderEditorGui(SailGuiWindow* window) {
	window->enableColumns();

	window->addProperty("Direction", [&] {
		if (ImGui::DragFloat3("##hideLabel", glm::value_ptr(m_direction), 0.005f)) {
			m_direction = glm::normalize(m_direction);
		}
	});
	window->addProperty("Color", [&] {
		ImGui::ColorEdit3("##hideLabel", glm::value_ptr(m_color));
	});
	window->addProperty("Intensity", [&] {
		ImGui::DragFloat("##hideLabel", &m_intensity, 0.1f, 0.f, 50.f);
	});

	window->disableColumns();
}
