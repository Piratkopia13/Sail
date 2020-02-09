#pragma once

#include "Component.h"
#include "Sail/graphics/material/Material.h"

#include "Sail/api/gui/SailGuiWindow.h"
//#include "imgui.h"
//#include "glm/gtc/type_ptr.hpp"
//#include "Sail/api/Texture.h"
//#include "Sail/graphics/geometry/Model.h"
//#include "Sail/utils/Utils.h"
#include "Sail/graphics/material/PhongMaterial.h"
#include "Sail/graphics/material/PBRMaterial.h"
//#include "Sail/graphics/material/TexturesMaterial.h"
//#include "imgui_internal.h"

template <typename T = Material>
class MaterialComponent : public Component {
public:
	SAIL_COMPONENT
	MaterialComponent()
		: m_textureFilter(L"All supported textures (*.tga;*.hdr;*.dds)\0*.tga;*.hdr;*.dds")
	{
		static_assert(std::is_base_of<Material, T>::value, "T must inherit from Material");
		m_material.reset(new T());
	}
	~MaterialComponent() { }

	T* get() {
		return m_material.get();
	}

	void renderEditorGui(SailGuiWindow* window) override {
		window->enableColumns();
		PhongMaterial* phongMat = nullptr;
		if (phongMat = dynamic_cast<PhongMaterial*>(m_material.get())) {
			renderPhongMaterialGui(window, phongMat);
			return;
		}

		window->disableColumns();
		window->enableColumns(120.f);
		PBRMaterial* pbrMat = nullptr;
		if (pbrMat = dynamic_cast<PBRMaterial*>(m_material.get())) {
			renderPBRMaterialGui(window, pbrMat);
			return;
		}
		window->disableColumns();

		ImGui::TextColored(ImVec4(0.8f, 0.2f, 0.2f, 1.0f), "Unknown material type");
	}

private:
	void renderPhongMaterialGui(SailGuiWindow* window, PhongMaterial* material) {
		window->addProperty("Ambient", [&] { ImGui::SliderFloat("##hideLabel", &material->getPhongSettings().ka, 0.f, 10.f); });
		window->addProperty("Diffuse", [&] { ImGui::SliderFloat("##hideLabel", &material->getPhongSettings().kd, 0.f, 10.f); });
		window->addProperty("Specular", [&] { ImGui::SliderFloat("##hideLabel", &material->getPhongSettings().ks, 0.f, 10.f); });

		window->addProperty("Shininess", [&] { ImGui::SliderFloat("##hideLabel", &material->getPhongSettings().shininess, 0.f, 100.f); });

		window->addProperty("Color", [&] { ImGui::ColorEdit4("##hideLabel", glm::value_ptr(material->getPhongSettings().modelColor)); });

		window->newSection("Textures");

		float trashButtonWidth = 21.f;
		std::string diffuseTexName = (material->getTexture(0)) ? material->getTexture(0)->getName() : "None - click to load";
		window->LimitStringLength(diffuseTexName);

		window->setOption("setWidth", false);
		window->addProperty("Diffuse", [&]() {
			float colWidth = ImGui::GetColumnWidth() - 10.f;
			if (ImGui::Button(diffuseTexName.c_str(), ImVec2(colWidth - trashButtonWidth - ImGui::GetStyle().ItemSpacing.x, 0))) {
				std::string filename = window->OpenFileDialog(m_textureFilter);
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
		window->LimitStringLength(normalTexName);
		window->addProperty("Normal", [&]() {
			float colWidth = ImGui::GetColumnWidth() - 10.f;
			if (ImGui::Button(normalTexName.c_str(), ImVec2(colWidth - trashButtonWidth - ImGui::GetStyle().ItemSpacing.x, 0))) {
				std::string filename = window->OpenFileDialog(m_textureFilter);
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
		window->LimitStringLength(specularTexName);
		window->addProperty("Specular", [&]() {
			float colWidth = ImGui::GetColumnWidth() - 10.f;
			if (ImGui::Button(specularTexName.c_str(), ImVec2(colWidth - trashButtonWidth - ImGui::GetStyle().ItemSpacing.x, 0))) {
				std::string filename = window->OpenFileDialog(m_textureFilter);
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

	void renderPBRMaterialGui(SailGuiWindow* window, PBRMaterial* material) {
		window->addProperty("Metalness scale", [&] { ImGui::SliderFloat("##hideLabel", &material->getPBRSettings().metalnessScale, 0.f, 1.f); });
		window->addProperty("Roughness scale", [&] { ImGui::SliderFloat("##hideLabel", &material->getPBRSettings().roughnessScale, 0.f, 1.f); });
		window->addProperty("AO intensity", [&] {
			ImGui::SliderFloat("##hideLabel", &material->getPBRSettings().aoIntensity, -0.7f, 0.7f);
		});

		window->addProperty("Color", [&] { ImGui::ColorEdit4("##hideLabel", glm::value_ptr(material->getPBRSettings().modelColor)); });

		window->newSection("Textures");

		float trashButtonWidth = 21.f;
		std::string diffuseTexName = (material->getTexture(0)) ? material->getTexture(0)->getName() : "None - click to load";
		window->LimitStringLength(diffuseTexName);

		window->setOption("setWidth", false);
		window->addProperty("Albedo", [&]() {
			float colWidth = ImGui::GetColumnWidth() - 10.f;
			if (ImGui::Button(diffuseTexName.c_str(), ImVec2(colWidth - trashButtonWidth - ImGui::GetStyle().ItemSpacing.x, 0))) {
				std::string filename = window->OpenFileDialog(m_textureFilter);
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
		window->LimitStringLength(normalTexName);
		window->addProperty("Normal", [&]() {
			float colWidth = ImGui::GetColumnWidth() - 10.f;
			if (ImGui::Button(normalTexName.c_str(), ImVec2(colWidth - trashButtonWidth - ImGui::GetStyle().ItemSpacing.x, 0))) {
				std::string filename = window->OpenFileDialog(m_textureFilter);
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
		window->LimitStringLength(specularTexName);
		window->addProperty("MRAO", [&]() {
			float colWidth = ImGui::GetColumnWidth() - 10.f;
			if (ImGui::Button(specularTexName.c_str(), ImVec2(colWidth - trashButtonWidth - ImGui::GetStyle().ItemSpacing.x, 0))) {
				std::string filename = window->OpenFileDialog(m_textureFilter);
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

private:
	const LPCWSTR m_textureFilter;
	std::shared_ptr<T> m_material;
};
