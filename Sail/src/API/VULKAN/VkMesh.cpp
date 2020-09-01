#include "pch.h"
#include "VkMesh.h"
#include "Sail/api/VertexBuffer.h"
#include "Sail/api/IndexBuffer.h"
#include "Sail/Application.h"
#include "Sail/api/shader/Shader.h"
//#include "resources/DescriptorHeap.h"
//#include "shader/VkShader.h"

Mesh* Mesh::Create(Data& buildData) {
	return SAIL_NEW VkMesh(buildData);
}

VkMesh::VkMesh(Data& buildData)
	: Mesh(buildData)
{
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	m_context = Application::getInstance()->getAPI<VkAPI>();

	// Create vertex buffer
	vertexBuffer = std::unique_ptr<VertexBuffer>(VertexBuffer::Create(buildData));
	// Create index buffer if indices are set
	if (buildData.numIndices > 0) {
		indexBuffer = std::unique_ptr<IndexBuffer>(IndexBuffer::Create(buildData));
	}
}

VkMesh::~VkMesh() {
	static_cast<VkAPI*>(Application::getInstance()->getAPI())->waitForGPU();
}

void VkMesh::draw(const Renderer& renderer, Material* material, Shader* shader, Environment* environment, void* cmdList) {
	assert("Mesh::draw Not implemented for Vulkan");
}
