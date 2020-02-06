#include "pch.h"
#include "MaterialComponent.h"

#include "Sail/api/gui/SailGuiWindow.h"
#include "imgui.h"
#include "glm/gtc/type_ptr.hpp"
#include "Sail/api/Texture.h"
#include "Sail/graphics/geometry/Model.h"
#include "Sail/graphics/material/PhongMaterial.h"
#include "Sail/graphics/material/PBRMaterial.h"
#include "Sail/utils/Utils.h"

SAIL_COMPONENT MaterialComponent::MaterialComponent(Material::Type type) {
	if (type == Material::PHONG)
		m_material.reset(new PhongMaterial());
	else if (type == Material::PBR)
		m_material.reset(new PBRMaterial());
	else
		Logger::Error("Shader requires unknown material type");
}

void MaterialComponent::renderEditorGui(SailGuiWindow* window) {
	PhongMaterial* phongMat = nullptr;
	if (phongMat = dynamic_cast<PhongMaterial*>(m_material.get())) {
		renderPhongMaterialGui(window, phongMat);
		return;
	}

	PBRMaterial* pbrMat = nullptr;
	if (pbrMat = dynamic_cast<PBRMaterial*>(m_material.get())) {
		renderPBRMaterialGui(window, pbrMat);
		return;
	}

	window->newSection("Unknown material type");
}

void MaterialComponent::renderPhongMaterialGui(SailGuiWindow* window, PhongMaterial* material) {
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

void MaterialComponent::renderPBRMaterialGui(SailGuiWindow* window, PBRMaterial* material) {
	window->addProperty("Metalness scale", [&] { ImGui::SliderFloat("##hideLabel", &material->getPBRSettings().metalnessScale, 0.f, 1.f); });
	window->addProperty("Roughness scale", [&] { ImGui::SliderFloat("##hideLabel", &material->getPBRSettings().roughnessScale, 0.f, 1.f); });
	window->addProperty("AO scale", [&] { ImGui::SliderFloat("##hideLabel", &material->getPBRSettings().aoScale, 0.f, 1.f); });

	window->addProperty("Color", [&] { ImGui::ColorEdit4("##hideLabel", glm::value_ptr(material->getPBRSettings().modelColor)); });

	window->newSection("Textures");

	float trashButtonWidth = 21.f;
	std::string diffuseTexName = (material->getTexture(0)) ? material->getTexture(0)->getName() : "None - click to load";
	window->limitStringLength(diffuseTexName);

	window->setOption("setWidth", false);
	window->addProperty("Albedo", [&]() {
		float colWidth = ImGui::GetColumnWidth() - 10.f;
		if (ImGui::Button(diffuseTexName.c_str(), ImVec2(colWidth - trashButtonWidth - ImGui::GetStyle().ItemSpacing.x, 0))) {
			std::string filename = window->openFileDialog(L"TGA textures (*.tga)\0*.tga");
			if (!filename.empty()) {
				material->setAlbedoTexture(filename, true);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_TRASH, ImVec2(trashButtonWidth, 0))) {
			material->setAlbedoTexture("");
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
	window->addProperty("MRAO", [&]() {
		float colWidth = ImGui::GetColumnWidth() - 10.f;
		if (ImGui::Button(specularTexName.c_str(), ImVec2(colWidth - trashButtonWidth - ImGui::GetStyle().ItemSpacing.x, 0))) {
			std::string filename = window->openFileDialog(L"TGA textures (*.tga)\0*.tga");
			if (!filename.empty()) {
				material->setMetalnessRoughnessAOTexture(filename, true);
			}
		}
		ImGui::SameLine();
		if (ImGui::Button(ICON_FA_TRASH, ImVec2(trashButtonWidth, 0))) {
			material->setMetalnessRoughnessAOTexture("");
		}
	});
	window->setOption("setWidth", true);
}
