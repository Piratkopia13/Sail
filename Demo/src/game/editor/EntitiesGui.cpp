#include "EntitiesGui.h"
#include "Sail.h"
#include "imgui_internal.h"
#include "Sail/graphics/material/PhongMaterial.h"

EntitiesGui::EntitiesGui() { }

void EntitiesGui::render(std::vector<Entity::SPtr>& entities) {
	newFrame();
	
	static int selectedEntityIndex = -1;
	Entity::SPtr selectedEntity = nullptr;
	{
		bool entityAddedThisFrame = false;
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 8.f));
		ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_NoScrollbar);
		ImGui::PopStyleVar(3);

		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetStyle().ItemSpacing.x);
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + ImGui::GetStyle().ItemSpacing.y);
		ImGui::AlignTextToFramePadding();
		ImGui::Text((std::to_string(entities.size()) + " entities in the world").c_str());
		ImGui::SameLine();
		const char* newEntityBtnText = "Add entity";
		ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - ImGui::CalcTextSize(newEntityBtnText).x - ImGui::GetStyle().ItemInnerSpacing.x * 2.f - ImGui::GetStyle().ItemSpacing.x);
		if (ImGui::Button(newEntityBtnText)) {
			selectedEntity = entities.emplace_back(Entity::Create("New entity"));
			entityAddedThisFrame = true;
		}


		if (ImGui::ListBoxHeader("##hideLabel", ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight() - 50.f))) {
			for (unsigned int i = 0; i < entities.size(); i++) {
				ImGui::PushID(i);
				auto& item = entities[i];
				const bool itemSelected = (i == selectedEntityIndex);
				const char* itemText = (item->getName() != "") ? item->getName().c_str() : "Unnamed";
				ImGui::SetCursorPosX(ImGui::GetStyle().ItemSpacing.x);
				if (ImGui::Selectable(itemText, itemSelected)) {
					selectedEntityIndex = i;
				}
				// Always select newly added entities
				if (entityAddedThisFrame && item == selectedEntity) {
					selectedEntityIndex = i;
					ImGui::SetScrollHere();
				}
				ImGui::SameLine();
				std::string hintText = std::to_string(item->getAllComponents().size()) + " components";
				float textWidth = ImGui::CalcTextSize(hintText.c_str()).x;
				ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - textWidth - ImGui::GetStyle().ItemSpacing.x);
				ImGui::TextDisabled(hintText.c_str());
				if (itemSelected)
					ImGui::SetItemDefaultFocus();
				ImGui::PopID();
			}
			ImGui::ListBoxFooter();
		}

		// Select the entity from the list if valid
		if (selectedEntityIndex < entities.size()) {
			selectedEntity = entities[selectedEntityIndex];
		}

		ImGui::End();
		//ImGui::PopStyleVar(3);

	}
	Component* selectedComponent = nullptr;
	{
		ImGui::Begin("Details");

		if (!selectedEntity) {
			ImGui::Text("Select an entity to view details");
			ImGui::End();
			return;
		}

		// Entity renaming and removing
		{
			float trashButtonWidth = 21.f;
			ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() - trashButtonWidth - ImGui::GetStyle().ItemSpacing.x);
			char buf[256];
			strcpy_s(buf, selectedEntity->getName().c_str());
			if (ImGui::InputTextWithHint("##entityName", "Entity name", buf, IM_ARRAYSIZE(buf), ImGuiInputTextFlags_AutoSelectAll)) {
				selectedEntity->setName(buf);
			}
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text("Rename the entity");
				ImGui::EndTooltip();
			}
			ImGui::SameLine();
			if (ImGui::Button(ICON_FA_TRASH, ImVec2(trashButtonWidth, 0))) {
				Logger::Log("Removed entity " + selectedEntity->getName());
				entities.erase(std::remove(entities.begin(), entities.end(), selectedEntity), entities.end());
				selectedEntityIndex = -1; // Unselect
			}
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.f), "Delete the entity");
				ImGui::EndTooltip();
			}
		}
		// Component count and add component button
		{
			ImGui::AlignTextToFramePadding();
			ImGui::Text((std::to_string(selectedEntity->getAllComponents().size()) + " components").c_str());
			ImGui::SameLine();
			const char* addComponentBtnText = "Add component";
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - ImGui::CalcTextSize(addComponentBtnText).x);
			ImVec2 popupPos;
			popupPos.x = ImGui::GetWindowPos().x + ImGui::GetCursorPos().x;
			ImGui::Button(addComponentBtnText);
			if (ImGui::IsItemClicked()) {
				ImGui::OpenPopup("ComponentList");
				popupPos.y = ImGui::GetWindowPos().y + ImGui::GetCursorPos().y;
				ImGui::SetNextWindowPos(popupPos);
			}
			if (ImGui::BeginPopup("ComponentList")) {
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.f, 0.f, 0.f, 0.f));
				int selectedNewComponentIndex = -1;
				const char* componentNames[] { "ModelComponent", "TransformComponent", "MaterialComponent" };
				if (ImGui::ListBox("##hideLabel", &selectedNewComponentIndex, componentNames, IM_ARRAYSIZE(componentNames))) {

					if (selectedNewComponentIndex == 0) {
						auto* defaultShader = &Application::getInstance()->getResourceManager().getShaderSet<PBRMaterialShader>();
						selectedEntity->addComponent<ModelComponent>(ModelFactory::CubeModel::Create(glm::vec3(0.5f), defaultShader));
					} else if (selectedNewComponentIndex == 1) {
						selectedEntity->addComponent<TransformComponent>();
					} else if (selectedNewComponentIndex == 2) {
						selectedEntity->addComponent<MaterialComponent>(Material::PBR);
					}

					ImGui::CloseCurrentPopup();
				}
				ImGui::PopStyleColor(1);
				ImGui::EndPopup();
			}
			
		}

		float w = ImGui::GetWindowContentRegionWidth();
		static float sz1 = 100.f;
		static float sz2 = 300.f;
		SailGuiWindow::DrawSplitter(false, 5.f, &sz1, &sz2, 20.f, 20.f, w);
		float adjustedSz1 = sz1 - ImGui::GetStyle().FramePadding.y;
		ImGui::BeginChild("Components", ImVec2(w, adjustedSz1));
		if (selectedEntity) {
			static int selectedIndex = -1;
			static int selectedComponentID = -1;
			if (ImGui::ListBoxHeader("##hideLabel", ImVec2(w, adjustedSz1))) {
				int i = 0;
				for (auto& item : selectedEntity->getAllComponents()) {
					ImGui::PushID(i);
					const bool itemSelected = (i == selectedIndex);
					std::string& itemText = item.second->getStaticName();
					ImGui::SetCursorPosX(ImGui::GetStyle().ItemSpacing.x);
					if (ImGui::Selectable(itemText.c_str(), itemSelected)) {
						selectedIndex = i;
						selectedComponentID = item.first;
					}
					ImGui::SameLine();
					std::string hintText = ICON_FA_SMILE_O;
					float textWidth = ImGui::CalcTextSize(hintText.c_str()).x;
					ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - textWidth - ImGui::GetStyle().ItemSpacing.x);
					ImGui::TextDisabled(hintText.c_str());
					if (itemSelected)
						ImGui::SetItemDefaultFocus();
					ImGui::PopID();
					i++;
				}
				ImGui::ListBoxFooter();
			}

			auto& val = selectedEntity->getAllComponents().find(selectedComponentID);
			if (val != selectedEntity->getAllComponents().end()) {
				selectedComponent = val->second.get();
			}
		}

		ImGui::EndChild();
		ImGui::BeginChild("Details", ImVec2(w, ImGui::GetWindowSize().y - sz1 + ImGui::GetStyle().FramePadding.y));

		ImGui::Separator();
		ImGui::Spacing(); ImGui::Spacing();
		if (selectedComponent) {
			selectedComponent->renderEditorGui(this);
		} else {
			ImGui::Text("Select a component to view its properties");
		}

		ImGui::EndChild();

		ImGui::End();
	}
}
