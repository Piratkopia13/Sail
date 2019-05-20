#include "pch.h"
#include "DX12Mesh.h"
#include "Sail/api/VertexBuffer.h"
#include "Sail/api/IndexBuffer.h"
#include "Sail/Application.h"
//#include "DX12API.h"

// TODO: only define if dx12 api is used
//Mesh* Mesh::Create(Data& buildData, ShaderSet* shaderSet) {
//	return SAIL_NEW DX12Mesh(buildData, shaderSet);
//}

DX12Mesh::DX12Mesh(Data& buildData, ShaderPipeline* shaderPipeline)
	: Mesh(buildData, shaderPipeline)
{

	//TODO: create DX11Index and Vertex buffer

	material = std::make_shared<Material>(shaderPipeline);
	// Create vertex buffer
	//vertexBuffer = std::make_unique<DX11VertexBuffer>(shaderSet->getInputLayout(), buildData);
	//// Create index buffer is indices are set
	//if (buildData.numIndices > 0) {
	//	indexBuffer = std::make_unique<DX11IndexBuffer>(buildData);
	//}
}

DX12Mesh::~DX12Mesh() {
}

void DX12Mesh::draw(const Renderer& renderer) {
	material->bind();

	vertexBuffer->bind();
	if (indexBuffer)
		indexBuffer->bind();

	// TODO: replace following DirectX calls with abstraction layer

	//auto* devCon = Application::getInstance()->getAPI<DX11API>()->getDeviceContext();
	//// Set topology
	//devCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	//// Draw call
	//if (indexBuffer)
	//	devCon->DrawIndexed(getNumIndices(), 0U, 0U);
	//else
	//	devCon->Draw(getNumVertices(), 0);
}
