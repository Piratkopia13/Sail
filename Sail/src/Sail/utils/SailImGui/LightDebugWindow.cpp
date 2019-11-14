#include "pch.h"

#include "LightDebugWindow.h"
#include "imgui.h"
#include "Sail/graphics/light/LightSetup.h"

LightDebugWindow::LightDebugWindow(bool showWindow)
	: m_lightSetup(nullptr)
	, m_manualOverride(false)
{}

LightDebugWindow::~LightDebugWindow() {

}

void LightDebugWindow::setLightSetup(LightSetup* lights) {
	m_lightSetup = lights;
}

bool LightDebugWindow::isManualOverrideOn() {
	return m_manualOverride;
}

void LightDebugWindow::setManualOverride(bool enable) {
	m_manualOverride = enable;
}

void LightDebugWindow::renderWindow() {
	ImGui::Begin("Light debug");
	ImGui::Checkbox("Manual override", &m_manualOverride);
	if (m_lightSetup) {
		unsigned int i = 0;
		for (auto& pl : m_lightSetup->getPLs()) {
			ImGui::PushID(i);
			std::string label("Point light ");
			label += std::to_string(i);
			if (ImGui::CollapsingHeader(label.c_str())) {

				glm::vec3 color = pl.getColor(); // = 1.0f
				glm::vec3 position = pl.getPosition(); // (12.f, 4.f, 0.f);
				float attConstant = pl.getAttenuation().constant; // 0.312f;
				float attLinear = pl.getAttenuation().linear; // 0.0f;
				float attQuadratic = pl.getAttenuation().quadratic; // 0.0009f;

				ImGui::SliderFloat3("Color##", &color[0], 0.f, 1.0f);
				ImGui::SliderFloat3("Position##", &position[0], -15.f, 15.0f);
				ImGui::SliderFloat("AttConstant##", &attConstant, 0.f, 1.f);
				ImGui::SliderFloat("AttLinear##", &attLinear, 0.f, 1.f);
				ImGui::SliderFloat("AttQuadratic##", &attQuadratic, 0.f, 0.2f);

				pl.setAttenuation(attConstant, attLinear, attQuadratic);
				pl.setColor(color);
				pl.setPosition(position);

			}
			i++;
			ImGui::PopID();
		}
	}
	ImGui::End();
}
