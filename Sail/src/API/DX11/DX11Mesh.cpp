#include "pch.h"
#include "DX11Mesh.h"
#include "DX11VertexBuffer.h"
#include "DX11IndexBuffer.h"
#include "Sail/Application.h"
#include "Sail/graphics/shader/Shader.h"
#include "DX11API.h"

Mesh* Mesh::Create(Data& buildData, Shader* shader) {
	return SAIL_NEW DX11Mesh(buildData, shader);
}

DX11Mesh::DX11Mesh(Data& buildData, Shader* shader) 
	: Mesh(buildData, shader)
{
	material = std::make_shared<Material>(shader);
	// Create vertex buffer
	vertexBuffer = std::unique_ptr<VertexBuffer>(VertexBuffer::Create(shader->getPipeline()->getInputLayout(), buildData));
	// Create index buffer if indices are set
	if (buildData.numIndices > 0) {
		indexBuffer = std::unique_ptr<IndexBuffer>(IndexBuffer::Create(buildData));
	}
}

DX11Mesh::~DX11Mesh() {
}

void DX11Mesh::draw(const Renderer& renderer, void* cmdList) {
	material->bind();

	vertexBuffer->bind();
	if (indexBuffer)
		indexBuffer->bind();

	auto* devCon = Application::getInstance()->getAPI<DX11API>()->getDeviceContext();
	// Set topology
	devCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// Draw call
	if (indexBuffer)
		devCon->DrawIndexed(getNumIndices(), 0U, 0U);
	else
		devCon->Draw(getNumVertices(), 0);
}
