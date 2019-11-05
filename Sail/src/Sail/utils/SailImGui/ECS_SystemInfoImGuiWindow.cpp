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
						if (ImGui::Selectable(std::string(e->getName()+"("+ std::to_string(e->getID())+")").c_str(), selectedEntity == e)) {
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
				if (TransformComponent * tc = selectedEntity->getComponent<TransformComponent>()) {
					std::type_index ti = typeid(TransformComponent);
					std::string name(ti.name());
					if (ImGui::CollapsingHeader(std::string(name.substr(name.find(" ")+1, std::string::npos)).c_str())) {
						ImGui::Text("Position"); ImGui::SameLine();
						glm::vec3 pos = tc->getTranslation();
						if (ImGui::DragFloat3("##allPos", &pos.x, 0.1f)) {
							tc->setTranslation(pos);
						}
						ImGui::Text("Rotation"); ImGui::SameLine();
						glm::vec3 rot = tc->getRotations();
						if (ImGui::DragFloat3("##allRot", &rot.x, 0.1f)) {
							tc->setRotations(rot);
						}
						ImGui::Text("Scale   "); ImGui::SameLine();
						glm::vec3 scale = tc->getScale();
						if (ImGui::DragFloat3("##allScale", &scale.x, 0.1f)) {
							tc->setScale(scale);
						}
					}
					
				}
				if (AnimationComponent* ac = selectedEntity->getComponent<AnimationComponent>()) {
					if (ImGui::CollapsingHeader(std::string("AnimationComponent").c_str())) {
						ImGui::Text("Index"); ImGui::SameLine();
						int index = ac->animationIndex;
						if (ImGui::DragInt("##aIndex", &index, 0.1f,0,ac->getAnimationStack()->getAnimationCount()-1)) {
							ac->setAnimation(index);
						}
					}
				}
				if (SpeedLimitComponent * spc = selectedEntity->getComponent<SpeedLimitComponent>()) {
					if (ImGui::CollapsingHeader(std::string("SpeedLimitComponent").c_str())) {
						ImGui::Text("Speed"); ImGui::SameLine();
						float speed = spc->maxSpeed;
						if (ImGui::DragFloat("##aIndex", &speed, 0.1f)) {
							spc->maxSpeed = speed;
						}
					}
				}
				if (GunComponent* gc = selectedEntity->getComponent<GunComponent>()) {
					if (ImGui::CollapsingHeader(std::string("GunComponent").c_str())) {
						ImGui::Text("Speed"); ImGui::SameLine();
						float bulletSpeed = gc->projectileSpeed;
						if (ImGui::DragFloat("##aspeeed", &bulletSpeed, 0.1f)) {
							gc->projectileSpeed = bulletSpeed;
						}
					}
				}
				if (CandleComponent * cc = selectedEntity->getComponent<CandleComponent>()) {
					if (ImGui::CollapsingHeader(std::string("CandleComponent").c_str())) {
						ImGui::Text("Health"); ImGui::SameLine();
						float value0 = cc->health;
						if (ImGui::DragFloat("##healthCandle", &value0, 0.1f)) {
							cc->health = value0;
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
