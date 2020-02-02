#include "pch.h"
#include "DX12Mesh.h"
#include "Sail/api/VertexBuffer.h"
#include "Sail/api/IndexBuffer.h"
#include "Sail/Application.h"
#include "Sail/graphics/shader/Shader.h"
#include "resources/DescriptorHeap.h"
#include "shader/DX12ShaderPipeline.h"

Mesh* Mesh::Create(Data& buildData, Shader* shader) {
	return SAIL_NEW DX12Mesh(buildData, shader);
}

DX12Mesh::DX12Mesh(Data& buildData, Shader* shader)
	: Mesh(buildData, shader)
{
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	m_context = Application::getInstance()->getAPI<DX12API>();
	material = std::make_shared<PhongMaterial>(shader);
	// Create vertex buffer
	vertexBuffer = std::unique_ptr<VertexBuffer>(VertexBuffer::Create(shader->getPipeline()->getInputLayout(), buildData));
	// Create index buffer if indices are set
	if (buildData.numIndices > 0) {
		indexBuffer = std::unique_ptr<IndexBuffer>(IndexBuffer::Create(buildData));
	}
}

DX12Mesh::~DX12Mesh() {
	static_cast<DX12API*>(Application::getInstance()->getAPI())->waitForGPU();
}

void DX12Mesh::draw(const Renderer& renderer, void* cmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);
	// Set offset in SRV heap for this mesh 
	dxCmdList->SetGraphicsRootDescriptorTable(m_context->getRootIndexFromRegister("t0"), m_context->getMainGPUDescriptorHeap()->getCurentGPUDescriptorHandle());

	material->bind(cmdList);

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
	static_cast<DX12ShaderPipeline*>(material->getShader()->getPipeline())->instanceFinished();
}
