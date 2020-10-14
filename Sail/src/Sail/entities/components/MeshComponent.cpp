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
		window->addProperty("Mesh", [&]() {
			std::string meshName = "Load new mesh";
			window->LimitStringLength(meshName, 30);
			if (ImGui::Button(meshName.c_str(), ImVec2(ImGui::GetColumnWidth(), 0))) {
				std::string newMesh = window->OpenFileDialog(L"FBX meshes (*.fbx)\0*.fbx");
				if (!newMesh.empty()) {
					// Wait for the GPU to finish rendering using the current mesh
					Application::getInstance()->getAPI()->waitForGPU();

					// Load new mesh and replace the old one
					auto& resman = Application::getInstance()->getResourceManager();
					m_mesh = resman.loadMesh(newMesh, true);
				}
			}
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ImGui::Text("Load a new mesh from file");
				ImGui::EndTooltip();
			}
		});
	window->disableColumns();
	
	ImGui::Text("Internal references to this mesh: %i", m_mesh.use_count());

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
