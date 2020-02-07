#include "pch.h"
#include "ModelComponent.h"
#include "Sail/api/gui/SailGuiWindow.h"
#include "Sail/graphics/geometry/Model.h"
#include "Sail/utils/Utils.h"
#include "Sail/Application.h"

ModelComponent::ModelComponent(Model::SPtr model)
	: m_model(model) 
{ }

Model::SPtr ModelComponent::getModel() {
	return m_model;
}

void ModelComponent::renderEditorGui(SailGuiWindow* window) {
	
	window->enableColumns();
	window->addProperty("Model", [&]() {
		std::string modelName = m_model->getName();
		window->LimitStringLength(modelName, 30);
		if (ImGui::Button(modelName.c_str(), ImVec2(ImGui::GetColumnWidth(), 0))) {
			std::string newModel = window->OpenFileDialog(L"FBX models (*.fbx)\0*.fbx");
			if (!newModel.empty()) {
				m_model = Application::getInstance()->getResourceManager().getModel(newModel, m_model->getMesh(0)->getShader(), true);
				Logger::Log("load a new model");
			}
		}
		if (ImGui::IsItemHovered()) {
			ImGui::BeginTooltip();
			ImGui::Text("Load a new model from file");
			ImGui::EndTooltip();
		}
	});

	window->disableColumns();
	ImGui::Text("Internal references to this model: %i", m_model.use_count());

	for (unsigned int i = 0; i < m_model->getNumberOfMeshes(); i++) {
		window->disableColumns();
		auto* mesh = m_model->getMesh(i);
		ImGui::SetNextTreeNodeOpen(true, ImGuiCond_Once);
		if (ImGui::TreeNode(("Mesh " + std::to_string(i)).c_str())) {
			window->enableColumns(150.f);
			window->addProperty("Vertex count", [&]() {
				ImGui::AlignTextToFramePadding();
				ImGui::Text(std::to_string(mesh->getNumVertices()).c_str());
			});
			window->addProperty("Index count", [&]() {
				ImGui::AlignTextToFramePadding();
				ImGui::Text(std::to_string(mesh->getNumIndices()).c_str());
			});
			window->addProperty("Instance count", [&]() {
				ImGui::AlignTextToFramePadding();
				ImGui::Text(std::to_string(mesh->getNumInstances()).c_str());
			});
			ImGui::TreePop();
		}
	}

	window->disableColumns();

}
