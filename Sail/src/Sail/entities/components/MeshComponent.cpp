#include "pch.h"
#include "MeshComponent.h"

#include "Sail/gui/SailGuiWindow.h"
#include "Sail/utils/Utils.h"
#include "Sail/Application.h"

MeshComponent::MeshComponent(Mesh::SPtr mesh)
	: m_mesh(mesh) 
{ }

Mesh* MeshComponent::get() {
	return m_mesh.get();
}

void MeshComponent::renderEditorGui(SailGuiWindow* window) {
	
	window->enableColumns();
		window->addProperty("Model", [&]() {
			std::string modelName = "This is a model name";
			window->LimitStringLength(modelName, 30);
			//if (ImGui::Button(modelName.c_str(), ImVec2(ImGui::GetColumnWidth(), 0))) {
			//	std::string newModel = window->OpenFileDialog(L"FBX models (*.fbx)\0*.fbx");
			//	if (!newModel.empty()) {
			//		// Wait for the GPU to finish rendering using the current model
			//		Application::getInstance()->getAPI()->waitForGPU();

			//		// Load new model and replace the old one
			//		m_model = Application::getInstance()->getResourceManager().getModel(newModel, true);
			//	}
			//}
			/*if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text("Load a new model from file");
				ImGui::EndTooltip();
			}*/
		});
	window->disableColumns();
	
	ImGui::Text("Internal references to this model: %i", m_mesh.use_count());

	if (m_mesh) {
		window->enableColumns(150.f);

		window->addProperty("Vertex count", [&]() {
			ImGui::AlignTextToFramePadding();
			ImGui::Text(std::to_string(m_mesh->getNumVertices()).c_str());
		});
		window->addProperty("Index count", [&]() {
			ImGui::AlignTextToFramePadding();
			ImGui::Text(std::to_string(m_mesh->getNumIndices()).c_str());
		});
		window->addProperty("Instance count", [&]() {
			ImGui::AlignTextToFramePadding();
			ImGui::Text(std::to_string(m_mesh->getNumInstances()).c_str());
		});

		window->disableColumns();
	}

}
