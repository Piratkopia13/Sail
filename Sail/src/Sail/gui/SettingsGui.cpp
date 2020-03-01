#include "pch.h"
#include "SettingsGui.h"
#include "imgui.h"
#include "../Application.h"

SettingsGui::SettingsGui() { }

void SettingsGui::render() {
	newFrame();

	if (ImGui::Begin("Settings")) {
		enableColumns();

		addProperty("SSAO", [&]() {
			bool enableSSAO = Application::getInstance()->getSettings().getBool(Settings::Graphics_SSAO);
			if (ImGui::Checkbox("##hideLabel", &enableSSAO)) {
				Application::getInstance()->getSettings().set(Settings::Graphics_SSAO, enableSSAO);
			}
		});

		addProperty("DXR", [&]() {
			bool enableDXR = Application::getInstance()->getSettings().getBool(Settings::Graphics_DXR);
			if (Application::getInstance()->getAPI()->supportsFeature(GraphicsAPI::RAYTRACING)) {
				if (ImGui::Checkbox("##hideLabel", &enableDXR)) {
					Application::getInstance()->getSettings().set(Settings::Graphics_DXR, enableDXR);
				}
			} else {
				ImGui::Text("Unsupported");
			}
		});

		disableColumns();
	}
	ImGui::End();
}
