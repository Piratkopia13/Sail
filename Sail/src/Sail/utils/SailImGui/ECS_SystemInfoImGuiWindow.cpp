#include "pch.h"
#include "ECS_SystemInfoImGuiWindow.h"
#include "imgui.h"

#include "../../entities/ECS.h"
#include "../../entities/components/Components.h"

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
#ifdef DEVELOPMENT
	ECS* ecs = ECS::Instance();
	if (!ecs) {
		return;
	}
	const ECS::SystemMap* systemMap = &ecs->getSystems();

	if (ImGui::Begin("ECS System Entities")) {
		static Entity* selectedEntity = nullptr;
		if(ImGui::BeginChild("SYSTEMS", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 0), false)) {
			for (auto const& [key, val] : *systemMap) {
				std::string name(key.name());

				if (ImGui::CollapsingHeader(std::string(name.substr(name.find(" "), std::string::npos) + ": " + std::to_string(val->getEntities().size())).c_str())) {
					for (const auto& e : val->getEntities()) {
						if (ImGui::Selectable(e->getName().c_str(), selectedEntity == e)) {
							selectedEntity = (selectedEntity == e ? nullptr : e);
							
						}
					}
				}
			}
		}
		ImGui::EndChild();
		ImGui::SameLine();
		if (ImGui::BeginChild("ENTITY", ImVec2(ImGui::GetWindowContentRegionWidth() * 0.5f, 0), false)) {
			if (selectedEntity) {
				ImGui::Text(std::string("Name: " + selectedEntity->getName()).c_str());
				ImGui::Text(std::string("ID: " + std::to_string(selectedEntity->getID())).c_str());
				if (selectedEntity->getParent()) {
					if (ImGui::Selectable(std::string("Parent: " + selectedEntity->getParent()->getName()).c_str(), selectedEntity == selectedEntity->getParent())) {
						selectedEntity = selectedEntity->getParent();
						ImGui::EndChild();
						ImGui::End();
						return;
					}

				}
				for (const auto& child : selectedEntity->getChildEntities()) {
					if (ImGui::Selectable(std::string("Child: " + child->getName()).c_str(), selectedEntity == child.get())) {
						selectedEntity = child.get();
						ImGui::EndChild();
						ImGui::End();
						return;
					}
				}
				ImGui::Separator();
				TransformComponent* tc = selectedEntity->getComponent<TransformComponent>();
				if (tc) {
					if (ImGui::CollapsingHeader(std::string("TransformComponent").c_str())) {
						ImGui::Text("Position");
						glm::vec3 pos = tc->getTranslation();
						bool changed = false; //?
						if (ImGui::DragFloat("##posx", &pos.x, 0.1f)) {
							changed = true;
						}
						if (ImGui::DragFloat("##posy", &pos.y, 0.1f)) {
							changed = true;
						}
						if (ImGui::DragFloat("##posz", &pos.z, 0.1f)) {
							changed = true;
						}
						if (changed) {
							tc->setTranslation(pos);
						}
					}
				}
				ImGui::Text("More to come..");
				//for (unsigned int i = 0; i < BaseComponent::nrOfComponentTypes(); i++) {
				//}




			}
		}
		ImGui::EndChild();

	}
	ImGui::End();
#endif
	return;
}
