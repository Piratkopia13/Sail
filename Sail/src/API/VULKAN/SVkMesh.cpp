#include "pch.h"
#include "SVkMesh.h"
#include "Sail/api/VertexBuffer.h"
#include "Sail/api/IndexBuffer.h"
#include "Sail/Application.h"
#include "Sail/api/shader/Shader.h"
#include "shader/SVkShader.h"

Mesh* Mesh::Create(Data& buildData) {
	return SAIL_NEW SVkMesh(buildData);
}

SVkMesh::SVkMesh(Data& buildData)
	: Mesh(buildData)
{
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	m_context = Application::getInstance()->getAPI<SVkAPI>();

	// Create vertex buffer
	vertexBuffer = std::unique_ptr<VertexBuffer>(VertexBuffer::Create(buildData));
	// Create index buffer if indices are set
	if (buildData.numIndices > 0) {
		indexBuffer = std::unique_ptr<IndexBuffer>(IndexBuffer::Create(buildData));
	}
}

SVkMesh::~SVkMesh() {
	static_cast<SVkAPI*>(Application::getInstance()->getAPI())->waitForGPU();
}

void SVkMesh::draw(const Renderer& renderer, Material* material, Shader* shader, Environment* environment, void* cmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	if (!shader) {
		Logger::Warning("Tried to draw mesh with no shader");
		return;
	}

	if (material && shader->getMaterialType() != material->getType()) {
		Logger::Warning("Shader requires a different material from the one given");
		return;
	}

	auto vkCmd = static_cast<VkCommandBuffer>(cmdList);

	if (material) {
		material->bind(shader, environment, cmdList);
	}
	// Write the new descriptors to bind textures
	static_cast<SVkShader*>(shader)->updateDescriptorSet(cmdList);
	shader->bind(cmdList);

	vertexBuffer->bind(cmdList);
	if (indexBuffer)
		indexBuffer->bind(cmdList);

	{
		SAIL_PROFILE_API_SPECIFIC_SCOPE("VK draw call");

		// Draw call
		if (indexBuffer)
			vkCmdDrawIndexed(vkCmd, getNumIndices(), 1, 0, 0, 0);
		else
			vkCmdDraw(vkCmd, getNumVertices(), 1, 0, 0);
	}

}
