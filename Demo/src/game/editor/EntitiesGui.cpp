#include "EntitiesGui.h"
#include "Sail.h"
#include "imgui_internal.h"
#include "Sail/graphics/material/PhongMaterial.h"

EntitiesGui::EntitiesGui() { }

void EntitiesGui::render(std::vector<Entity::SPtr>& entities) {
	newFrame();
	
	Entity* selectedEntity = nullptr;
	{
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 0.f);
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.x, 8.f));
		ImGui::Begin("Entities", nullptr, ImGuiWindowFlags_NoScrollbar);

		static int selectedIndex = -1;
		if (ImGui::ListBoxHeader("##hideLabel", ImGui::GetWindowSize())) {
			for (unsigned int i = 0; i < entities.size(); i++) {
				ImGui::PushID(i);
				auto& item = entities[i];
				const bool itemSelected = (i == selectedIndex);
				const char* itemText = (item->getName() != "") ? item->getName().c_str() : "Unnamed";
				if (ImGui::Selectable(itemText, itemSelected)) {
					selectedIndex = i;
				}
				ImGui::SameLine();
				std::string hintText = std::to_string(item->getAllComponents().size()) + " components";
				float textWidth = ImGui::CalcTextSize(hintText.c_str()).x;
				ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - textWidth);
				ImGui::TextDisabled(hintText.c_str());
				if (itemSelected)
					ImGui::SetItemDefaultFocus();
				ImGui::PopID();
			}
			ImGui::ListBoxFooter();
		}

		if (selectedIndex < entities.size()) {
			selectedEntity = entities[selectedIndex].get();
		}

		ImGui::End();
		ImGui::PopStyleVar(3);

	}
	Component* selectedComponent = nullptr;
	{
		ImGui::Begin("Details");

		if (!selectedEntity) {
			ImGui::Text("Select an entity to view details");
			ImGui::End();
			return;
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
					if (ImGui::Selectable(itemText.c_str(), itemSelected)) {
						selectedIndex = i;
						selectedComponentID = item.first;
					}
					ImGui::SameLine();
					std::string hintText = ICON_FA_SMILE_O;
					float textWidth = ImGui::CalcTextSize(hintText.c_str()).x;
					ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - textWidth);
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
