#include "pch.h"
#include "DX12Mesh.h"
#include "Sail/api/VertexBuffer.h"
#include "Sail/api/IndexBuffer.h"
#include "Sail/Application.h"
#include "Sail/api/shader/ShaderPipeline.h"
#include "DX12API.h"

Mesh* Mesh::Create(Data& buildData, ShaderPipeline* shaderPipeline) {
	return SAIL_NEW DX12Mesh(buildData, shaderPipeline);
}

DX12Mesh::DX12Mesh(Data& buildData, ShaderPipeline* shaderPipeline)
	: Mesh(buildData, shaderPipeline)
{
	material = std::make_shared<Material>(shaderPipeline);
	// Create vertex buffer
	vertexBuffer = std::unique_ptr<VertexBuffer>(VertexBuffer::Create(shaderPipeline->getInputLayout(), buildData));
	// Create index buffer if indices are set
	if (buildData.numIndices > 0) {
		indexBuffer = std::unique_ptr<IndexBuffer>(IndexBuffer::Create(buildData));
	}
}

DX12Mesh::~DX12Mesh() {
}

void DX12Mesh::draw(const Renderer& renderer, void* cmdList) {
	material->bind(cmdList);

	vertexBuffer->bind(cmdList);
	if (indexBuffer)
		indexBuffer->bind(cmdList);

	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);
	// Draw call
	if (indexBuffer)
		dxCmdList->DrawIndexedInstanced(getNumIndices(), 1, 0, 0, 0);
	else
		dxCmdList->DrawInstanced(getNumVertices(), 1, 0, 0);
}
