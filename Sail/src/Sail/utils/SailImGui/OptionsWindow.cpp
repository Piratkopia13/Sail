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
#ifdef DEVELOPMENT
	ImGui::DragFloat4("##POS", &x[0], 1.0f);
#endif
	SettingStorage::Setting* sopt = nullptr;
	SettingStorage::DynamicSetting* dopt = nullptr;
	unsigned int selected = 0;
	std::string valueName = "";
	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();
	// GRAPHICS
	ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
	ImGui::Text("graphics");
	ImGui::PopStyleColor();
	ImGui::Separator();



	static std::vector<std::string> options = { "fullscreen","bloom","shadows","fxaa","water simulation" };
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
	


		//sopt = &stat["graphics"]["fxaa"];
		//selected = sopt->selected;
		//if (SailImGui::TextButton(std::string("<##fxaa").c_str())) {
		//	sopt->setSelected(selected - 1);
		//}
		//ImGui::SameLine(x[0]);
		//valueName = sopt->getSelected().name;
		//ImGui::Text(valueName.c_str());
		//ImGui::SameLine(x[1]);
		//if (SailImGui::TextButton(std::string(">##fxaa").c_str())) {
		//	sopt->setSelected(selected + 1);
		//}
		//ImGui::SameLine(x[2]);
		//ImGui::Text("FXAA");



		//sopt = &stat["graphics"]["shadows"];
		//selected = sopt->selected;
		//if (SailImGui::TextButton(std::string("<##shadows").c_str())) {
		//	sopt->setSelected(selected - 1);
		//}
		//ImGui::SameLine(x[0]);
		//valueName = sopt->getSelected().name;
		//ImGui::Text(valueName.c_str());
		//ImGui::SameLine(x[1]);
		//if (SailImGui::TextButton(std::string(">##shadows").c_str())) {
		//	sopt->setSelected(selected + 1);
		//}
		//ImGui::SameLine(x[2]);
		//ImGui::Text("shadows");

		//sopt = &stat["graphics"]["water simulation"];
		//selected = sopt->selected;
		//if (SailImGui::TextButton(std::string("<##water simulation").c_str())) {
		//	sopt->setSelected(selected - 1);
		//}
		//ImGui::SameLine(x[0]);
		//valueName = sopt->getSelected().name;
		//ImGui::Text(valueName.c_str());
		//ImGui::SameLine(x[1]);
		//if (SailImGui::TextButton(std::string(">##water simulation").c_str())) {
		//	sopt->setSelected(selected + 1);
		//}
		//ImGui::SameLine(x[2]);
		//ImGui::Text("water simulation");



	ImGui::Spacing();
	ImGui::Spacing();
	ImGui::Spacing();
	// SOUND
	ImGui::PushStyleColor(ImGuiCol_Text, ImGui::GetStyleColorVec4(ImGuiCol_ButtonHovered));
	ImGui::Text("Sound");
	ImGui::PopStyleColor();
	ImGui::Separator();

	const static std::vector<std::string> soundSettings = { "global", "music", "effects", "voices" };
	for (auto& settingName : soundSettings) {
		dopt = &dynamic["sound"][settingName];
		ImGui::Text(settingName.c_str());
		ImGui::SameLine(x[0]);
		ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() * 0.5f);
		float val = dopt->value * 100.0f;
		if (ImGui::SliderFloat(std::string("##"+settingName).c_str(), &val, dopt->minVal, dopt->maxVal * 100.0f, "%.1f%%")) {
			dopt->setValue(val * 0.01f);
		}


	}


		

}

bool OptionsWindow::renderGameOptions() {

	static float x[4] = {
	ImGui::GetWindowContentRegionWidth() * 0.5f,
	0,
	ImGui::GetWindowContentRegionWidth() * 0.95f,
	0
	};
	ImGui::DragFloat4("##POS", &x[0], 1.0f);

	bool settingsChanged = false;
	static auto& dynamic = m_app->getSettings().gameSettingsDynamic;
	static auto& stat = m_app->getSettings().gameSettingsStatic;
	ImGui::Text("Map Settings");
	ImGui::Separator();
	ImGui::Columns(2);
	ImGui::Text("Setting"); ImGui::NextColumn();
	ImGui::Text("Value"); ImGui::NextColumn();
	ImGui::Separator();

	SettingStorage::DynamicSetting* mapSizeX = &m_app->getSettings().gameSettingsDynamic["map"]["sizeX"];
	SettingStorage::DynamicSetting* mapSizeY = &m_app->getSettings().gameSettingsDynamic["map"]["sizeY"];

	static int size[] = { 0,0 };
	size[0] = (int)mapSizeX->value;
	size[1] = (int)mapSizeY->value;
	ImGui::Text("MapSize"); ImGui::NextColumn();
	if (ImGui::SliderInt2("##MapSizeXY", size, (int)mapSizeX->minVal, (int)mapSizeX->maxVal)) {
		mapSizeX->value = size[0];
		mapSizeY->value = size[1];
		settingsChanged = true;
	}
	ImGui::NextColumn();

	int seed = dynamic["map"]["seed"].value;
	ImGui::Text("Seed"); ImGui::NextColumn();
	if (ImGui::InputInt("##SEED", &seed)) {
		dynamic["map"]["seed"].setValue(seed);
		settingsChanged = true;
	}
	ImGui::NextColumn();
	ImGui::Text("Clutter"); ImGui::NextColumn();
	float val = m_app->getSettings().gameSettingsDynamic["map"]["clutter"].value;
	if (ImGui::SliderFloat("##Clutter",
		&val,
		m_app->getSettings().gameSettingsDynamic["map"]["clutter"].minVal,
		m_app->getSettings().gameSettingsDynamic["map"]["clutter"].maxVal
	)) {
		m_app->getSettings().gameSettingsDynamic["map"]["clutter"].setValue(val);
		settingsChanged = true;
	}
	ImGui::NextColumn();

	ImGui::Columns(1);
	ImGui::Text("Gamemode Settings");
	ImGui::Separator();
	ImGui::Columns(2);

	SettingStorage::Setting* sopt = nullptr;
	SettingStorage::DynamicSetting* dopt = nullptr;
	unsigned int selected = 0;
	std::string valueName = "";

	sopt = &stat["gamemode"]["types"];
	selected = sopt->selected;
	if (SailImGui::TextButton(std::string("<##gamemode").c_str())) {
		sopt->setSelected(selected - 1);
		settingsChanged = true;
	}
	ImGui::SameLine();
	valueName = sopt->getSelected().name;
	ImGui::Text(valueName.c_str());
	ImGui::SameLine();
	if (SailImGui::TextButton(std::string(">##gamemode").c_str())) {
		sopt->setSelected(selected + 1);
		settingsChanged = true;
	}
	//ImGui::SameLine();
	ImGui::NextColumn();
	ImGui::Text("gamemode");



	ImGui::Columns(1);
	return settingsChanged;
}
