#include "pch.h"
#include "DX12Mesh.h"
#include "Sail/api/VertexBuffer.h"
#include "Sail/api/IndexBuffer.h"
#include "Sail/Application.h"
#include "Sail/api/shader/Shader.h"
#include "resources/DescriptorHeap.h"
#include "shader/DX12Shader.h"

Mesh* Mesh::Create(Data& buildData) {
	return SAIL_NEW DX12Mesh(buildData);
}

DX12Mesh::DX12Mesh(Data& buildData)
	: Mesh(buildData)
{
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	m_context = Application::getInstance()->getAPI<DX12API>();

	// Create vertex buffer
	vertexBuffer = std::unique_ptr<VertexBuffer>(VertexBuffer::Create(buildData));
	// Create index buffer if indices are set
	if (buildData.numIndices > 0) {
		indexBuffer = std::unique_ptr<IndexBuffer>(IndexBuffer::Create(buildData));
	}
}

DX12Mesh::~DX12Mesh() {
	static_cast<DX12API*>(Application::getInstance()->getAPI())->waitForGPU();
}

void DX12Mesh::draw(const Renderer& renderer, Material* material, Shader* shader, Environment* environment, void* cmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	if (!shader) {
		Logger::Warning("Tried to draw mesh with no shader");
		return;
	}

	if (material && shader->getMaterialType() != material->getType()) {
		Logger::Warning("Shader requires a different material from the one given");
		return;
	}

	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);
	// Set offset in SRV heap for this mesh 
	dxCmdList->SetGraphicsRootDescriptorTable(m_context->getRootSignEntryFromRegister("t0").rootSigIndex, m_context->getMainGPUDescriptorHeap()->getCurentGPUDescriptorHandle());

	if (material) {
		material->bind(shader, environment, cmdList);
	}

	vertexBuffer->bind(cmdList);
	if (indexBuffer)
		indexBuffer->bind(cmdList);

	{
		SAIL_PROFILE_API_SPECIFIC_SCOPE("DX12 draw call");

		// Draw call
		if (indexBuffer)
			dxCmdList->DrawIndexedInstanced(getNumIndices(), 1, 0, 0, 0);
		else
			dxCmdList->DrawInstanced(getNumVertices(), 1, 0, 0);
	}

	// Update pipeline mesh index to not overwrite this instance cbuffer data
	static_cast<DX12Shader*>(shader)->instanceFinished();
}
