#include "EntitiesGui.h"
#include "Sail.h"
#include "imgui_internal.h"
#include "Sail/graphics/material/PhongMaterial.h"
#include <unordered_map>
#include "Sail/entities/components/Component.h"
#include "Sail/resources/loaders/ModelLoader.h"

EntitiesGui::EntitiesGui() {
	// Register components that should show up in the component list
	registerComponent<TransformComponent>();
	registerComponent<MaterialComponent>();
	registerComponent<MeshComponent>();
	registerComponent<DirectionalLightComponent>();
	registerComponent<PointLightComponent>();

#define REGISTER_GUI_COMPONENT(T) \
if (componentID == entt::type_info<T>::id()) \
	return e.tryGetComponent<T>()

	// Add a line for each component that inherit Component and should be rendered in the GUI
	m_getComponentInstanceFromID = [](Entity& e, ENTT_ID_TYPE componentID) -> Component* {
		REGISTER_GUI_COMPONENT(TransformComponent);
		REGISTER_GUI_COMPONENT(MaterialComponent);
		REGISTER_GUI_COMPONENT(MeshComponent);
		REGISTER_GUI_COMPONENT(DirectionalLightComponent);
		REGISTER_GUI_COMPONENT(PointLightComponent);

		Logger::Error("Component instance not registered in EntitiesGui");
		return nullptr;
	};
}

void EntitiesGui::addComponent(Entity& entity, const char* componentName) {
	if (strcmp(componentName, "MeshComponent") == 0) {
		entity.addOrReplaceComponent<MeshComponent>(MeshFactory::Cube::Create(glm::vec3(0.5f)));

	} else if (strcmp(componentName, "TransformComponent") == 0) {
		entity.addOrReplaceComponent<TransformComponent>();

	} else if (strcmp(componentName, "PointLightComponent") == 0) {
		entity.addOrReplaceComponent<PointLightComponent>();

	} else if (strcmp(componentName, "DirectionalLightComponent") == 0) {
		entity.addOrReplaceComponent<DirectionalLightComponent>();

	} else {
		Logger::Warning("Tried to add a component that is unknown to EntitiesGui");
	}
}

void EntitiesGui::removeComponent(Entity& entity, const char* componentName) {
	if (strcmp(componentName, "MeshComponent") == 0) {
		entity.removeComponent<MeshComponent>();

	} else if (strcmp(componentName, "TransformComponent") == 0) {
		entity.removeComponent<TransformComponent>();

	} else if (strcmp(componentName, "PointLightComponent") == 0) {
		entity.removeComponent<PointLightComponent>();

	} else if (strcmp(componentName, "DirectionalLightComponent") == 0) {
		entity.removeComponent<DirectionalLightComponent>();

	} else if (strcmp(componentName, "MaterialComponent") == 0) {
		entity.removeComponent<MaterialComponent>();

	} else {
		Logger::Warning("Tried to remove a component that is unknown to EntitiesGui");
	}
}

void EntitiesGui::addMaterialComponent(Entity& entity, const char* materialName) {
	if (strcmp(materialName, "PBR") == 0) {
		entity.addOrReplaceComponent<MaterialComponent>().getAs<PBRMaterial>();

	} else if (strcmp(materialName, "Phong") == 0) {
		entity.addOrReplaceComponent<MaterialComponent>().getAs<PhongMaterial>();

	} else {
		Logger::Warning("Tried to add a material that is unknown to EntitiesGui");
	}
}

void EntitiesGui::render(Scene* scene) {
	newFrame();
	static Entity selectedEntity;
	static Entity::ID selectedEntityID;
	static Component* selectedComponent = nullptr;
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
		ImGui::Text((std::to_string(scene->getEntityCount()) + " entities in the world").c_str());
		ImGui::SameLine();

		const char* newEntityBtnText = "Add entity";
		auto newEntityPosX = ImGui::GetWindowContentRegionWidth() - ImGui::CalcTextSize(newEntityBtnText).x - ImGui::GetStyle().ItemInnerSpacing.x * 2.f - ImGui::GetStyle().ItemSpacing.x;
		const char* loadModelBtnText = "Add model";
		auto loadModelPosX = newEntityPosX - ImGui::CalcTextSize(loadModelBtnText).x - ImGui::GetStyle().ItemInnerSpacing.x * 2.f - ImGui::GetStyle().ItemSpacing.x;
		{
			ImGui::SetCursorPosX(loadModelPosX);
			if (ImGui::Button(loadModelBtnText)) {
				auto filter = L"All supported model files (*.fbx;*.dae;*.glb;*.blend;*.3ds;*.ase;*.obj;*.ifc;*.zgl;*.ply;*.dxf;*.lwo;*.lws;*.lxo;*.stl;*.x;*.ac;*.ms3d;*.scn;*.bvh;*.csm;*.xml;*.irrmesh;*.irr;*.mdl;*.md2;*.md3;*.pk3;*.mdc;*.md5*;*.vta;*.ogex;*.3d;*.b3d;*.q3s;*.nff;*.nff;*.off;*.raw;*.ter;*.mdl;*.hmp;*.ndo)\0*.fbx;*.dae;*.glb;*.blend;*.3ds;*.ase;*.obj;*.ifc;*.zgl;*.ply;*.dxf;*.lwo;*.lws;*.lxo;*.stl;*.x;*.ac;*.ms3d;*.scn;*.bvh;*.csm;*.xml;*.irrmesh;*.irr;*.mdl;*.md2;*.md3;*.pk3;*.mdc;*.md5*;*.vta;*.ogex;*.3d;*.b3d;*.q3s;*.nff;*.nff;*.off;*.raw;*.ter;*.mdl;*.hmp;*.ndo";
				std::string modelPath = OpenFileDialog(filter);
				if (!modelPath.empty()) {
					// Load the model and add it to the scene
					ModelLoader(modelPath, scene, true);
				}
			}
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text("Load a model scene from file");
				ImGui::EndTooltip();
			}
			ImGui::SameLine();
		}
		ImGui::SetCursorPosX(newEntityPosX);
		if (ImGui::Button(newEntityBtnText)) {
			selectEntity(scene->createEntity("New entity"), scene);
			entityAddedThisFrame = true;
		}


		if (ImGui::ListBoxHeader("##hideLabel", ImVec2(ImGui::GetWindowWidth(), ImGui::GetWindowHeight() - 50.f))) {
			uint32_t index = 0;
			scene->getEnttRegistry().each([&](auto& entity) {
				Entity e(entity, scene);
				auto* relation = e.tryGetComponent<RelationshipComponent>();

				// Don't list entities with parents, since these have already been, or will be, listed
				if (!relation || !relation->parent) {
					listEntity(e, &index, &selectedEntityID, entityAddedThisFrame);
				}
			});
			ImGui::ListBoxFooter();
		}

		// Select the entity from the list if valid
		if (selectedEntityID) {
			selectEntity(selectedEntityID, scene);
		}

		ImGui::End();
		//ImGui::PopStyleVar(3);

	}
	selectedEntity = Entity(selectedEntityID, scene);

	// Components window
	{
		ImGui::Begin("Details");

		if (!m_selectedEntityID) {
			ImGui::Text("Select an entity to view details");
			ImGui::End();
			return;
		}

		Entity selected(m_selectedEntityID, scene);

		// Entity renaming and removing
		{
			ImGui::SetNextItemWidth(ImGui::GetWindowContentRegionWidth() - trashButtonWidth - ImGui::GetStyle().ItemSpacing.x);
			char buf[256];
			strcpy_s(buf, selected.getName().c_str());
			if (ImGui::InputTextWithHint("##entityName", "Entity name", buf, IM_ARRAYSIZE(buf), ImGuiInputTextFlags_AutoSelectAll)) {
				selected.setName(buf);
			}
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text("Rename the entity");
				ImGui::EndTooltip();
			}
			ImGui::SameLine();
			if (ImGui::Button(ICON_FA_TRASH, ImVec2(trashButtonWidth, 0))) {
				std::string entityName = selected.getName();
				// Remove the entity from the scene
				scene->destroyEntity(selectedEntity);
				
				Logger::Log("Removed entity " + entityName);

				// Deselect
				m_selectedEntityID = Entity::ID();
				selectedEntityID = Entity::ID();

				// There is no entity to render details about, just return
				ImGui::End();
				return;
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
			ImGui::Text((std::to_string(selected.size()) + " components").c_str());
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
				for (auto& componentName : m_componentNameList) {
					if (componentName == "MaterialComponent") {
						if (ImGui::BeginMenu(componentName.c_str())) {
							for (auto& matName : m_materialNames) {
								if (ImGui::MenuItem(matName)) {
									addMaterialComponent(selectedEntity, matName);
								}
							}
							ImGui::EndMenu();
						}
					} else {
						if (ImGui::MenuItem(componentName.c_str())) {
							addComponent(selectedEntity, componentName.c_str());
						}
					}
				}
				ImGui::EndPopup();
			}

		}

		float w = ImGui::GetWindowContentRegionWidth();
		static float sz1 = 100.f;
		static float sz2 = 300.f;
		SailGuiWindow::DrawSplitter(false, 3.f, &sz1, &sz2, 20.f, 20.f, w);
		float adjustedSz1 = sz1 - ImGui::GetStyle().FramePadding.y;
		ImGui::BeginChild("Components", ImVec2(w, adjustedSz1));

		static ENTT_ID_TYPE selectedComponentID = 0;
		if (selected) {
			if (ImGui::ListBoxHeader("##hideLabel", ImVec2(w, adjustedSz1))) {

				int i = 0;
				scene->getEnttRegistry().visit(selected, [&](const auto componentID) {

					auto it = m_componentNames.find(componentID);
					if (it == m_componentNames.end()) return;

					auto componentName = it->second;
					
					ImGui::PushID(i);
					const bool itemSelected = (selectedComponentID == componentID);
					std::string& itemText = componentName;
					ImGui::SetCursorPosX(ImGui::GetStyle().ItemSpacing.x);
					if (ImGui::Selectable(itemText.c_str(), itemSelected)) {
						selectedComponentID = componentID;
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

				});

				ImGui::ListBoxFooter();
			}
			if (selectedComponentID) {
				selectedComponent = m_getComponentInstanceFromID(selectedEntity, selectedComponentID);
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
				auto& componentName = m_componentNames[selectedComponentID];
				Logger::Log("Removed component " + componentName);
				removeComponent(selectedEntity, componentName.c_str());
				selectedComponentID = 0; // Deselect
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

void EntitiesGui::selectEntity(Entity::ID entity, Scene* scene) {
	if (m_selectedEntityID) {
		Entity(m_selectedEntityID, scene).setIsSelected(false);
	}
	m_selectedEntityID = entity;
	if (m_selectedEntityID) {
		Entity(m_selectedEntityID, scene).setIsSelected(true);
	}
}

void EntitiesGui::listEntity(Entity& e, uint32_t* index, Entity::ID* pSelectedEntity, bool entityAddedThisFrame) {
	ImGui::PushID(*index);
	const bool itemSelected = (e == *pSelectedEntity);
	std::string eName = e.getName();
	const char* itemText = eName.c_str();
	ImGui::SetCursorPosX(ImGui::GetCursorPosX() + ImGui::GetStyle().ItemSpacing.x);

	// Always select newly added entities
	if (entityAddedThisFrame && e == m_selectedEntityID) {
		*pSelectedEntity = e;
		ImGui::SetScrollHere();
	}


	size_t numChildren = 0;
	auto relation = e.tryGetComponent<RelationshipComponent>();
	if (relation) {
		numChildren = relation->numChildren;
	}
	bool drawChildren = false;

	if (numChildren == 0) {
		// Draw a selectable, update selected entity if clicked
		if (ImGui::Selectable(itemText, itemSelected)) {
			*pSelectedEntity = e;
		}
	} else {
		ImGuiTreeNodeFlags flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick | ImGuiTreeNodeFlags_SpanAvailWidth;
		flags |= (itemSelected) ? ImGuiTreeNodeFlags_Selected : 0;
		drawChildren = ImGui::TreeNodeEx(itemText, flags);
		// Update selected entity if clicked
		if (ImGui::IsItemClicked()) {
			*pSelectedEntity = e;
		}
	}

	ImGui::SameLine();
	// Draw warning icon if entity is not rendered for some reason
	if (!e.isBeingRendered()) {
		ImGui::TextColored(ImVec4(0.8f, 0.8f, 0.2f, 1.f), ICON_FA_EXCLAMATION_TRIANGLE);
		if (ImGui::IsItemHovered()) {
			ImGui::SetNextWindowSize(ImVec2(200.f, 0.f));
			ImGui::BeginTooltip();
			ImGui::TextWrapped("Entity is not being drawn in the scene, it might be missing required components");
			ImGui::EndTooltip();
		}
		ImGui::SameLine();
	}

	std::string hintText = std::to_string(e.size()) + " components";
	float textWidth = ImGui::CalcTextSize(hintText.c_str()).x;
	ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - textWidth - ImGui::GetStyle().ItemSpacing.x);
	ImGui::TextDisabled(hintText.c_str());

	if (drawChildren) {
		// Draw children recursively
		Entity curr(relation->first, e.getScene());
		for (size_t i = 0; i < numChildren; ++i) {
			listEntity(curr, index, pSelectedEntity, entityAddedThisFrame); // Recursive call

			curr = Entity(curr.getComponent<RelationshipComponent>().next, e.getScene());
		}
		ImGui::TreePop();
	}
	
	if (itemSelected)
		ImGui::SetItemDefaultFocus();
	ImGui::PopID();

	(*index)++;
}
