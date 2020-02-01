#include "pch.h"
#include "ModelComponent.h"

#include "Sail/api/gui/SailGuiWindow.h"
#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "Sail/graphics/geometry/PhongMaterial.h"
#include "Sail/graphics/geometry/Model.h"

void ModelComponent::renderEditorGui(SailGuiWindow* window) {
	PhongMaterial* material = m_model->getMesh(0)->getMaterial();

	window->addProperty("Ambient", [&] { ImGui::SliderFloat("##hideLabel", &material->getPhongSettings().ka, 0.f, 10.f); });
	window->addProperty("Diffuse", [&] { ImGui::SliderFloat("##hideLabel", &material->getPhongSettings().kd, 0.f, 10.f); });
	window->addProperty("Specular", [&] { ImGui::SliderFloat("##hideLabel", &material->getPhongSettings().ks, 0.f, 10.f); });

	window->addProperty("Shininess", [&] { ImGui::SliderFloat("##hideLabel", &material->getPhongSettings().shininess, 0.f, 100.f); });

	window->addProperty("Color", [&] { ImGui::ColorEdit4("##hideLabel", glm::value_ptr(material->getPhongSettings().modelColor)); });

	window->newSection("Textures");

	float trashButtonWidth = 21.f;
	std::string diffuseTexName = (material->getTexture(0)) ? material->getTexture(0)->getName() : "None - click to load";
	window->limitStringLength(diffuseTexName);

	window->setOption("setWidth", false);
	window->addProperty("Diffuse", [&]() {
		float colWidth = ImGui::GetColumnWidth() - 10.f;
		if (ImGui::Button(diffuseTexName.c_str(), ImVec2(colWidth - trashButtonWidth - ImGui::GetStyle().ItemSpacing.x, 0))) {
			std::string filename = window->openFileDialog(L"TGA textures (*.tga)\0*.tga");
			if (!filename.empty()) {
				material->setDiffuseTexture(filename, true);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_TRASH, ImVec2(trashButtonWidth, 0))) {
			material->setDiffuseTexture("");
		}
	});

	std::string normalTexName = (material->getTexture(1)) ? material->getTexture(1)->getName() : "None - click to load";
	window->limitStringLength(normalTexName);
	window->addProperty("Normal", [&]() {
		float colWidth = ImGui::GetColumnWidth() - 10.f;
		if (ImGui::Button(normalTexName.c_str(), ImVec2(colWidth - trashButtonWidth - ImGui::GetStyle().ItemSpacing.x, 0))) {
			std::string filename = window->openFileDialog(L"TGA textures (*.tga)\0*.tga");
			if (!filename.empty()) {
				material->setNormalTexture(filename, true);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_TRASH, ImVec2(trashButtonWidth, 0))) {
			material->setNormalTexture("");
		}
	});

	std::string specularTexName = (material->getTexture(2)) ? material->getTexture(2)->getName() : "None - click to load";
	window->limitStringLength(specularTexName);
	window->addProperty("Specular", [&]() {
		float colWidth = ImGui::GetColumnWidth() - 10.f;
		if (ImGui::Button(specularTexName.c_str(), ImVec2(colWidth - trashButtonWidth - ImGui::GetStyle().ItemSpacing.x, 0))) {
			std::string filename = window->openFileDialog(L"TGA textures (*.tga)\0*.tga");
			if (!filename.empty()) {
				material->setSpecularTexture(filename, true);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_TRASH, ImVec2(trashButtonWidth, 0))) {
			material->setSpecularTexture("");
		}
	});
	window->setOption("setWidth", true);
}
