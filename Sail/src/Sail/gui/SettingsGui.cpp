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

		disableColumns();
	}
	ImGui::End();
}
