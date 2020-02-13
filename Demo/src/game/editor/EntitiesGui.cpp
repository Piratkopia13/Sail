#include "EntitiesGui.h"
#include "Sail.h"
#include "imgui_internal.h"
#include "Sail/graphics/material/PhongMaterial.h"

EntitiesGui::EntitiesGui() { }

void EntitiesGui::render(std::vector<Entity::SPtr>& entities) {
	newFrame();
	
	static int selectedEntityIndex = -1;
	Component* selectedComponent = nullptr;
	float trashButtonWidth = 21.f;

	// Entities window
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
			selectEntity(entities.emplace_back(Entity::Create("New entity")));
			entityAddedThisFrame = true;
		}


		if (ImGui::ListBoxHeader("##hideLabel", ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight() - 50.f))) {
			for (unsigned int i = 0; i < entities.size(); i++) {
				ImGui::PushID(i);
				auto& item = entities[i];
				const bool itemSelected = (i == selectedEntityIndex);
				const char* itemText = (item->getName() != "") ? item->getName().c_str() : "Unnamed";
				ImGui::SetCursorPosX(ImGui::GetStyle().ItemSpacing.x);
				
				// Draw warning icon if entity is not rendered for some reason
				if (!item->isBeingRendered()) { 
					ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.f), ICON_FA_EXCLAMATION_TRIANGLE);
					if (ImGui::IsItemHovered()) {
						ImGui::SetNextWindowSize(ImVec2(200.f, 0.f));
						ImGui::BeginTooltip();
						ImGui::TextWrapped("Entity is not being drawn in the scene, it might be missing required components");
						ImGui::EndTooltip();
					}
					ImGui::SameLine();
				}

				if (ImGui::Selectable(itemText, itemSelected)) {
					selectedEntityIndex = i;
				}
				// Always select newly added entities
				if (entityAddedThisFrame && item == m_selectedEntity) {
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
			selectEntity(entities[selectedEntityIndex]);
		}

		ImGui::End();
		//ImGui::PopStyleVar(3);

	}

	// Components window
	{
		ImGui::Begin("Details");

		if (!m_selectedEntity) {
			ImGui::Text("Select an entity to view details");
			ImGui::End();
			return;
		}

		// Entity renaming and removing
		{
			ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() - trashButtonWidth - ImGui::GetStyle().ItemSpacing.x);
			char buf[256];
			strcpy_s(buf, m_selectedEntity->getName().c_str());
			if (ImGui::InputTextWithHint("##entityName", "Entity name", buf, IM_ARRAYSIZE(buf), ImGuiInputTextFlags_AutoSelectAll)) {
				m_selectedEntity->setName(buf);
			}
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text("Rename the entity");
				ImGui::EndTooltip();
			}
			ImGui::SameLine();
			if (ImGui::Button(ICON_FA_TRASH, ImVec2(trashButtonWidth, 0))) {
				Logger::Log("Removed entity " + m_selectedEntity->getName());
				entities.erase(std::remove(entities.begin(), entities.end(), m_selectedEntity), entities.end());
				selectedEntityIndex = -1; // Deselect
			}
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.f), "Delete this entity");
				ImGui::EndTooltip();
			}
		}
		// Component count and add component button
		{
			ImGui::AlignTextToFramePadding();
			ImGui::Text((std::to_string(m_selectedEntity->getAllComponents().size()) + " components").c_str());
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
				for (unsigned int i = 0; i < AddableComponent::Size; i++) {
					if (i == AddableComponent::MaterialComponent) {
						if (ImGui::BeginMenu(m_componentNames[i])) {
							for (unsigned int j = 0; j < AddableMaterial::Size; j++) {
								if (ImGui::MenuItem(m_materialNames[j])) {
									addMaterialComponent((AddableMaterial::Type)j);
								}
							}
							ImGui::EndMenu();
						}
					} else {
						if (ImGui::MenuItem(m_componentNames[i])) {
							addComponent((AddableComponent::Type)i);
						}
					}
				}
				ImGui::EndPopup();
			}
			
		}

		static int selectedComponentIndex = -1;
		float w = ImGui::GetWindowContentRegionWidth();
		static float sz1 = 100.f;
		static float sz2 = 300.f;
		SailGuiWindow::DrawSplitter(false, 3.f, &sz1, &sz2, 20.f, 20.f, w);
		float adjustedSz1 = sz1 - ImGui::GetStyle().FramePadding.y;
		ImGui::BeginChild("Components", ImVec2(w, adjustedSz1));
		if (m_selectedEntity) {
			static int selectedComponentID = -1;
			if (ImGui::ListBoxHeader("##hideLabel", ImVec2(w, adjustedSz1))) {
				int i = 0;
				for (auto& item : m_selectedEntity->getAllComponents()) {
					ImGui::PushID(i);
					const bool itemSelected = (i == selectedComponentIndex);
					std::string& itemText = item.second->getName();
					ImGui::SetCursorPosX(ImGui::GetStyle().ItemSpacing.x);
					if (ImGui::Selectable(itemText.c_str(), itemSelected)) {
						selectedComponentIndex = i;
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

			auto& val = m_selectedEntity->getAllComponents().find(selectedComponentID);
			if (val != m_selectedEntity->getAllComponents().end()) {
				selectedComponent = val->second.get();
			}
		}

		ImGui::EndChild();
		ImGui::BeginChild("Details", ImVec2(w, 0));

		ImGui::Separator();
		ImGui::Spacing(); ImGui::Spacing();
		
		if (selectedComponent) {
			// Render component properties
			selectedComponent->renderEditorGui(this);

			// Remove component button
			ImGui::Separator();
			if (ImGui::Button(ICON_FA_TRASH, ImVec2(trashButtonWidth, 0))) {
				Logger::Log("Removed a component with id " + std::to_string(selectedComponent->getID()));
				m_selectedEntity->removeComponentByID(selectedComponent->getID());
				selectedComponentIndex = -1; // Deselect
				selectedComponent = nullptr;
			}
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.f), "Delete this component");
				ImGui::EndTooltip();
			}
		} else {
			ImGui::Text("Select a component to view its properties");
		}

		ImGui::EndChild();

		ImGui::End();
	}
}

void EntitiesGui::selectEntity(Entity::SPtr entity) {
	if (m_selectedEntity) {
		m_selectedEntity->setIsSelectedInGui(false);
	}
	m_selectedEntity = entity;
	m_selectedEntity->setIsSelectedInGui(true);
}

void EntitiesGui::addComponent(AddableComponent::Type comp) {
	auto* defaultShader = &Application::getInstance()->getResourceManager().getShaderSet(Shaders::PBRMaterialShader);

	switch (comp) {
	case AddableComponent::ModelComponent:
		m_selectedEntity->addComponent<ModelComponent>(ModelFactory::CubeModel::Create(glm::vec3(0.5f), defaultShader));
		break;
	case AddableComponent::TransformComponent:
		m_selectedEntity->addComponent<TransformComponent>();
		break;
	case AddableComponent::PointLightComponent:
		m_selectedEntity->addComponent<PointLightComponent>();
		break;
	case AddableComponent::DirectionalLightComponent:
		m_selectedEntity->addComponent<DirectionalLightComponent>();
		break;
	}
}

void EntitiesGui::addMaterialComponent(AddableMaterial::Type comp) {
	switch (comp) {
	case AddableMaterial::PBRMaterial:
		m_selectedEntity->addComponent<MaterialComponent<PBRMaterial>>();
		break;
	case AddableMaterial::PhongMaterial:
		m_selectedEntity->addComponent<MaterialComponent<PhongMaterial>>();
		break;
	case AddableMaterial::TexturesMaterial:
		m_selectedEntity->addComponent<MaterialComponent<TexturesMaterial>>();
		break;
	case AddableMaterial::OutlineMaterial:
		m_selectedEntity->addComponent<MaterialComponent<OutlineMaterial>>();
		break;
	}
}
