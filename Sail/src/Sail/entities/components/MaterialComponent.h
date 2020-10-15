#pragma once

#include "Component.h"
#include "Sail/graphics/material/Material.h"

#include "Sail/gui/SailGuiWindow.h"
#include "Sail/graphics/material/PhongMaterial.h"
#include "Sail/graphics/material/PBRMaterial.h"
#include "Sail/graphics/material/OutlineMaterial.h"
#include "Sail/api/Texture.h"

class MaterialComponent : public Component {
public:
	MaterialComponent() { };
	MaterialComponent(std::shared_ptr<Material> mat) : m_material(mat) { };

	Material* get() {
		assert(m_material && "Instance not created, make sure to call getAs<Type> before get()");
		return m_material.get();
	}

	template<typename MaterialType>
	MaterialType* getAs() {
		if (!m_material) {
			m_material = std::make_shared<MaterialType>();
		}
		return static_cast<MaterialType*>(m_material.get());
	}

	void renderEditorGui(SailGuiWindow* window) override {
		window->enableColumns();
		if (PhongMaterial* phongMat = dynamic_cast<PhongMaterial*>(m_material.get())) {
			renderPhongMaterialGui(window, phongMat);
			return;
		}
		if (OutlineMaterial* outlineMat = dynamic_cast<OutlineMaterial*>(m_material.get())) {
			renderOutlineMaterialGui(window, outlineMat);
			return;
		}

		window->disableColumns();
		if (PBRMaterial* pbrMat = dynamic_cast<PBRMaterial*>(m_material.get())) {
			renderPBRMaterialGui(window, pbrMat);
			return;
		}

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
				std::string filename = window->OpenFileDialog(s_textureFilter);
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
				std::string filename = window->OpenFileDialog(s_textureFilter);
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
				std::string filename = window->OpenFileDialog(s_textureFilter);
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
		window->enableColumns(120.f);

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

		//window->setOption("setWidth", false);
		window->addProperty("Albedo", [&]() {
			float colWidth = ImGui::GetColumnWidth() - 10.f;
			if (ImGui::Button(diffuseTexName.c_str(), ImVec2(colWidth - trashButtonWidth - ImGui::GetStyle().ItemSpacing.x, 0))) {
				std::string filename = window->OpenFileDialog(s_textureFilter);
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
				std::string filename = window->OpenFileDialog(s_textureFilter);
				if (!filename.empty()) {
					material->setNormalTexture(filename, true);
				}
			}
			ImGui::SameLine();
			if (ImGui::Button(ICON_FA_TRASH, ImVec2(trashButtonWidth, 0))) {
				material->setNormalTexture("");
			}
		});

		window->disableColumns();
		window->enableColumns(100.f);
		static bool useSeparateMrao = false;
		window->addProperty("Separate MRAO", [&]() {
			ImGui::SetCursorPosX(ImGui::GetWindowContentRegionWidth() - 55.f);
			ImGui::Checkbox("", &useSeparateMrao);
		});
		window->disableColumns();
		window->enableColumns();

		if (useSeparateMrao) {

			std::string metalnessTexName = (material->getTexture(6)) ? material->getTexture(6)->getName() : "None - click to load";
			window->LimitStringLength(metalnessTexName);
			window->addProperty("Metalness", [&]() {
				float colWidth = ImGui::GetColumnWidth() - 10.f;
				if (ImGui::Button(metalnessTexName.c_str(), ImVec2(colWidth - trashButtonWidth - ImGui::GetStyle().ItemSpacing.x, 0))) {
					std::string filename = window->OpenFileDialog(s_textureFilter);
					if (!filename.empty()) {
						material->setMetalnessTexture(filename, true);
					}
				}
				ImGui::SameLine();
				if (ImGui::Button(ICON_FA_TRASH, ImVec2(trashButtonWidth, 0))) {
					material->setMetalnessTexture("");
				}
			});
			std::string roughnessTexName = (material->getTexture(7)) ? material->getTexture(7)->getName() : "None - click to load";
			window->LimitStringLength(roughnessTexName);
			window->addProperty("Roughness", [&]() {
				float colWidth = ImGui::GetColumnWidth() - 10.f;
				if (ImGui::Button(roughnessTexName.c_str(), ImVec2(colWidth - trashButtonWidth - ImGui::GetStyle().ItemSpacing.x, 0))) {
					std::string filename = window->OpenFileDialog(s_textureFilter);
					if (!filename.empty()) {
						material->setRoughnessTexture(filename, true);
					}
				}
				ImGui::SameLine();
				if (ImGui::Button(ICON_FA_TRASH, ImVec2(trashButtonWidth, 0))) {
					material->setRoughnessTexture("");
				}
			});
			std::string aoTexName = (material->getTexture(8)) ? material->getTexture(8)->getName() : "None - click to load";
			window->LimitStringLength(aoTexName);
			window->addProperty("AO", [&]() {
				float colWidth = ImGui::GetColumnWidth() - 10.f;
				if (ImGui::Button(aoTexName.c_str(), ImVec2(colWidth - trashButtonWidth - ImGui::GetStyle().ItemSpacing.x, 0))) {
					std::string filename = window->OpenFileDialog(s_textureFilter);
					if (!filename.empty()) {
						material->setAoTexture(filename, true);
					}
				}
				ImGui::SameLine();
				if (ImGui::Button(ICON_FA_TRASH, ImVec2(trashButtonWidth, 0))) {
					material->setAoTexture("");
				}
			});

		} else {
			std::string mraoTexName = (material->getTexture(2)) ? material->getTexture(2)->getName() : "None - click to load";
			window->LimitStringLength(mraoTexName);
			window->addProperty("MRAO", [&]() {
				float colWidth = ImGui::GetColumnWidth() - 10.f;
				if (ImGui::Button(mraoTexName.c_str(), ImVec2(colWidth - trashButtonWidth - ImGui::GetStyle().ItemSpacing.x, 0))) {
					std::string filename = window->OpenFileDialog(s_textureFilter);
					if (!filename.empty()) {
						material->setMetalnessRoughnessAOTexture(filename, true);
					}
				}
				ImGui::SameLine();
				if (ImGui::Button(ICON_FA_TRASH, ImVec2(trashButtonWidth, 0))) {
					material->setMetalnessRoughnessAOTexture("");
				}
			});
		}

		//window->setOption("setWidth", true);

		window->disableColumns();
	}
	void renderOutlineMaterialGui(SailGuiWindow* window, OutlineMaterial* material) {
		window->addProperty("Color", [&] { 
			glm::vec3 color = material->getColor();
			if (ImGui::ColorEdit3("##hideLabel", glm::value_ptr(color))) {
				material->setColor(color);
			}
		});
		window->addProperty("Thickness", [&] { 
			float thickness = material->getThickness();
			if (ImGui::SliderFloat("##hideLabel", &thickness, 0.f, 0.2f)) {
				material->setThickness(thickness);
			}
		});
	}
private:
	inline static const LPCWSTR s_textureFilter = L"All supported textures (*.tga;*.hdr;*.dds;*.ktx;*.jpg;*.jpeg;*.png;*.bpm;*.psd;*.gif;*.pic)\0*.tga;*.hdr;*.dds;*.ktx;*.jpg;*.jpeg;*.png;*.bpm;*.psd;*.gif;*.pic";
	std::shared_ptr<Material> m_material;
};
