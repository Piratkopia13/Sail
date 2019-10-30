#include "pch.h"
#include "ECS_SystemInfoImGuiWindow.h"
#include "imgui.h"

ECS_SystemInfoImGuiWindow::ECS_SystemInfoImGuiWindow(bool showWindow) {
}

ECS_SystemInfoImGuiWindow::~ECS_SystemInfoImGuiWindow() {
}

void ECS_SystemInfoImGuiWindow::updateNumEntitiesInSystems(std::string systemName, int n) {
	m_nEntitiesInSystems[systemName] = n;
}

void ECS_SystemInfoImGuiWindow::updateNumEntitiesInECS(int n) {
	m_nEntitiesInECS = n;
}

void ECS_SystemInfoImGuiWindow::renderWindow() {
	ImGui::Begin("ECS entity count");
	

	int i = 0;

	ImGui::PushID(i);
	std::string label("");
	label += "ECS";
	label += " : ";
	label += std::to_string(m_nEntitiesInECS);
	ImGui::Text(label.c_str());
	i++;
	ImGui::PopID();

	ImGui::Separator();

	for (auto e : m_nEntitiesInSystems) {
		ImGui::PushID(i);
		std::string label("");
		label += e.first;
		label += " : ";
		label += std::to_string(e.second);
		ImGui::Text(label.c_str());
		i++;
		ImGui::PopID();
	}

	ImGui::End();
}
