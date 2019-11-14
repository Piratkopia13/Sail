#include "pch.h"
#include "OptionsWindow.h"
#include "Sail/Application.h"
#include "Sail/KeyBinds.h"
#include "Sail/utils/SailImGui/SailImGui.h"

OptionsWindow::OptionsWindow(bool showWindow) {
	m_app = Application::getInstance();
	m_settings = &m_app->getSettings();
}

OptionsWindow::~OptionsWindow() {}

void OptionsWindow::renderWindow() {
	// Rendering a pause window in the middle of the game window.
	
	auto& stat = m_settings->applicationSettingsStatic;
	auto& dynamic = m_settings->applicationSettingsDynamic;

	static float x[4] = { 
		ImGui::GetWindowContentRegionWidth()*0.5f,
		0,
		ImGui::GetWindowContentRegionWidth() * 0.95f,
		0 
	};


	ImGui::DragFloat4("##POS", &x[0], 1.0f);

	SettingStorage::Setting* sopt = nullptr;
	SettingStorage::DynamicSetting* dopt = nullptr;
	unsigned int selected = 0;
	std::string valueName = "";
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();
	// GRAPHICS
	ImGui::Text("graphics");
	ImGui::Separator();



	static std::vector<std::string> options = { "fullscreen","bloom","shadows" };
	for (auto & optionName : options) {
		sopt = &stat["graphics"][optionName];
		selected = sopt->selected;
		ImGui::Text(optionName.c_str());
		ImGui::SameLine(x[0]);
		if (SailImGui::TextButton(std::string("<##"+ optionName).c_str())) {
			sopt->setSelected(selected - 1);
		}
		ImGui::SameLine(x[1]);
		valueName = sopt->getSelected().name;
		SailImGui::cText(valueName.c_str(), x[2]);
		ImGui::SameLine(x[2]);
		if (SailImGui::TextButton(std::string(">##"+ optionName).c_str())) {
			sopt->setSelected(selected + 1);
		}
	}
	



	


	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();
	// SOUND
	ImGui::Text("Sound");
	ImGui::Separator();

	dopt = &dynamic["sound"]["global"];
	ImGui::SliderFloat(std::string("##Global").c_str(), &dopt->value, dopt->minVal, dopt->maxVal);
	ImGui::SameLine();
	ImGui::Text("global");


	dopt = &dynamic["sound"]["music"];
	ImGui::SliderFloat(std::string("##music").c_str(), &dopt->value, dopt->minVal, dopt->maxVal);
	ImGui::SameLine();
	ImGui::Text("music");

	dopt = &dynamic["sound"]["effects"];
	ImGui::SliderFloat(std::string("##effects").c_str(), &dopt->value, dopt->minVal, dopt->maxVal);
	ImGui::SameLine();
	ImGui::Text("effects");

	dopt = &dynamic["sound"]["voices"];
	ImGui::SliderFloat(std::string("##voices").c_str(), &dopt->value, dopt->minVal, dopt->maxVal);
	ImGui::SameLine();
	ImGui::Text("voices");

		

}