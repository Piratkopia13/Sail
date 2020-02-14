#include "pch.h"
#include "PointLightComponent.h"

#include "Sail/gui/SailGuiWindow.h"
#include "imgui.h"

PointLightComponent::PointLightComponent() 
	: m_color(glm::vec3(1.f))
	, m_position(glm::vec3(0.f))
	, m_attenuationRadius(10.f)
	, m_intensity(10.f)
{ }

PointLightComponent::~PointLightComponent() { }

void PointLightComponent::setColor(const glm::vec3& color) {
	m_color = color;
}

const glm::vec3& PointLightComponent::getColor() const {
	return m_color;
}

void PointLightComponent::setPosition(const glm::vec3& position) {
	m_position = position;
}

const glm::vec3& PointLightComponent::getPosition() const {
	return m_position;
}

void PointLightComponent::setAttenuationRadius(float radius) {
	m_attenuationRadius = radius;
}

float PointLightComponent::getAttenuationRadius() const {
	return m_attenuationRadius;
}

void PointLightComponent::setIntensity(float intensity) {
	m_intensity = intensity;
}

float PointLightComponent::getIntensity() {
	return m_intensity;
}

void PointLightComponent::renderEditorGui(SailGuiWindow* window) {
	window->enableColumns();

	window->addProperty("Position", [&] {
		ImGui::DragFloat3("##hideLabel", glm::value_ptr(m_position), 0.05f);
	});
	window->addProperty("Color", [&] {
		ImGui::ColorEdit3("##hideLabel", glm::value_ptr(m_color));
	});
	window->addProperty("Intensity", [&] {
		ImGui::DragFloat("##hideLabel", &m_intensity, 0.1f, 0.f, 50.f);
	});

	window->disableColumns();
	window->enableColumns(150.f);

	window->addProperty("Attenuation radius", [&] {
		ImGui::DragFloat("##hideLabel", &m_attenuationRadius, 0.1f, 0.f, 50.f);
	});

	window->disableColumns();
}
