#include "DynBlockDeferredInstancedGeometryShader.h"

using namespace DirectX;
using namespace SimpleMath;

D3D11_INPUT_ELEMENT_DESC DynBlockDeferredInstancedGeometryShader::IED[11] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "MODELMAT", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "MODELMAT", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "MODELMAT", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "MODELMAT", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "VARIATION_OFFSET", 0, DXGI_FORMAT_R32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 }
};

DynBlockDeferredInstancedGeometryShader::DynBlockDeferredInstancedGeometryShader()
	: m_clippingPlaneHasChanged(false) {

	// Set up constant buffers
	ModelDataBuffer defaultModelData = { Matrix::Identity, Matrix::Identity, Vector4::One, 1.f, 1.f, 1.f, 10.f, 0, 0, 0 };
	m_modelDataBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&defaultModelData, sizeof(ModelDataBuffer)));
	WorldDataBuffer defaultworldData = { Vector4::Zero };
	m_worldDataBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&defaultworldData, sizeof(WorldDataBuffer)));

	// Set up sampler
	m_sampler = std::make_unique<ShaderComponent::Sampler>();

	// Compile VS
	auto vsBlob = compileShader(L"deferred/DynBlockInstancedGeometryShader.hlsl", "VSMain", "vs_5_0");
	// Add the VS to this ShaderSet
	setVertexShader(vsBlob);

	// Compile GS
	auto gsBlob = compileShader(L"deferred/DynBlockInstancedGeometryShader.hlsl", "GSMain", "gs_5_0");
	// Add the GS to this ShaderSet
	setGeometryShader(gsBlob);

	// Compile PS
	auto psBlob = compileShader(L"deferred/DynBlockInstancedGeometryShader.hlsl", "PSMain", "ps_5_0");
	// Add the PS to this ShaderSet
	setPixelShader(psBlob);

	// Create the input layouts
	Application::getInstance()->getDXManager()->getDevice()->CreateInputLayout(IED, 11, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);

	// Done with the blobs, release them
	Memory::safeRelease(vsBlob);
	Memory::safeRelease(psBlob);

}
DynBlockDeferredInstancedGeometryShader::~DynBlockDeferredInstancedGeometryShader() {
	Memory::safeRelease(m_inputLayout);
}

void DynBlockDeferredInstancedGeometryShader::updateCamera(Camera& cam) {
	m_mView = cam.getViewMatrix();
	m_mProjection = cam.getProjMatrix();
}

void DynBlockDeferredInstancedGeometryShader::setClippingPlane(const DirectX::SimpleMath::Vector4& clippingPlane) {
	m_clippingPlane = clippingPlane;
	m_clippingPlaneHasChanged = true;
}

void DynBlockDeferredInstancedGeometryShader::updateWorldDataBuffer(const DirectX::SimpleMath::Vector4& clippingPlaneVS) const {
	WorldDataBuffer data = { clippingPlaneVS };
	m_worldDataBuffer->updateData(&data, sizeof(data));
}

void DynBlockDeferredInstancedGeometryShader::updateModelDataBuffer(const Material& material, const DirectX::SimpleMath::Matrix& v, const DirectX::SimpleMath::Matrix& p) const {

	const bool* texFlags = material.getTextureFlags();

	ModelDataBuffer data = {
		v.Transpose(), p.Transpose() ,
		material.getColor(),
		material.getKa(),
		material.getKd(),
		material.getKs(),
		material.getShininess(),
	{ (int)texFlags[0], (int)texFlags[1], (int)texFlags[2] }
	};

	m_modelDataBuffer->updateData(&data, sizeof(data));
}

void DynBlockDeferredInstancedGeometryShader::updateInstanceData(const void* instanceData, UINT bufferSize, ID3D11Buffer* instanceBuffer) {

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	Application::getInstance()->getDXManager()->getDeviceContext()->Map(instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	std::memcpy(mappedResource.pData, instanceData, bufferSize);
	Application::getInstance()->getDXManager()->getDeviceContext()->Unmap(instanceBuffer, 0);

}

void DynBlockDeferredInstancedGeometryShader::bind() {

	// Call parent to bind shaders
	ShaderSet::bind();

	// Bind cbuffers
	m_modelDataBuffer->bind(ShaderComponent::VS, 0);
	m_modelDataBuffer->bind(ShaderComponent::PS, 0);
	m_worldDataBuffer->bind(ShaderComponent::GS, 0);
	// Bind sampler
	m_sampler->bind();

}

void DynBlockDeferredInstancedGeometryShader::createBufferFromModelData(ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, ID3D11Buffer** instanceBuffer, const void* data) {

	Model::Data& modelData = *(Model::Data*)data;

	if (modelData.numVertices <= 0 || !modelData.positions)
		Logger::Error("numVertices or position data not set for model");

	if (!modelData.texCoords)
		Logger::Warning("Texture coordinates not set for model that will render with a texture shader");

	// Create the vertex array that this shader uses
	DynBlockDeferredInstancedGeometryShader::Vertex* vertices = new DynBlockDeferredInstancedGeometryShader::Vertex[modelData.numVertices];

	for (UINT i = 0; i < modelData.numVertices; i++) {
		// Position
		vertices[i].position = modelData.positions[i];
		// Tex coords
		if (modelData.texCoords)
			vertices[i].texCoords = modelData.texCoords[i];
		else
			vertices[i].texCoords = Vector2::Zero;
		// Normals
		if (modelData.normals)
			vertices[i].normal = modelData.normals[i];
		else
			vertices[i].normal = Vector3::Zero;
		// Tangents
		if (modelData.tangents)
			vertices[i].tangent = modelData.tangents[i];
		else
			vertices[i].tangent = Vector3::Zero;
		// Bitangents
		if (modelData.bitangents)
			vertices[i].bitangent = modelData.bitangents[i];
		else
			vertices[i].bitangent = Vector3::Zero;
	}

	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));

	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = modelData.numVertices * sizeof(DynBlockDeferredInstancedGeometryShader::Vertex);
	vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	vbd.CPUAccessFlags = 0;

	D3D11_SUBRESOURCE_DATA vertexData;
	ZeroMemory(&vertexData, sizeof(vertexData));
	vertexData.pSysMem = vertices;

	// Create the vertex buffer
	ThrowIfFailed(Application::getInstance()->getDXManager()->getDevice()->CreateBuffer(&vbd, &vertexData, vertexBuffer));
	// Delete vertices from cpu memory
	Memory::safeDeleteArr(vertices);

	// Set up index buffer if indices are set
	if (modelData.numIndices > 0) {

		ULONG* indices = new ULONG[modelData.numIndices];

		// Fill the array with the model indices
		for (UINT i = 0; i < modelData.numIndices; i++) {
			indices[i] = modelData.indices[i];
		}

		// Set up index buffer description
		D3D11_BUFFER_DESC ibd;
		ZeroMemory(&ibd, sizeof(ibd));
		ibd.Usage = D3D11_USAGE_IMMUTABLE;
		ibd.ByteWidth = modelData.numIndices * sizeof(UINT);
		ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		ibd.CPUAccessFlags = 0;

		D3D11_SUBRESOURCE_DATA indexData;
		ZeroMemory(&indexData, sizeof(indexData));
		indexData.pSysMem = indices;

		// Create the index buffer
		ThrowIfFailed(Application::getInstance()->getDXManager()->getDevice()->CreateBuffer(&ibd, &indexData, indexBuffer));
		// Delete indices from cpu memory
		Memory::safeDeleteArr(indices);
	}

	// Set up instanceData buffer if instances are set
	if (modelData.numInstances > 0) {
		// Set up instance buffer
		D3D11_BUFFER_DESC ibd;
		ZeroMemory(&ibd, sizeof(ibd));
		ibd.Usage = D3D11_USAGE_DYNAMIC;
		ibd.ByteWidth = sizeof(InstanceData) * modelData.numInstances;
		ibd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		ibd.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
		ibd.MiscFlags = 0;
		ibd.StructureByteStride = 0;

		ThrowIfFailed(Application::getInstance()->getDXManager()->getDevice()->CreateBuffer(&ibd, nullptr, instanceBuffer));
	}

}

void DynBlockDeferredInstancedGeometryShader::draw(Model& model, bool bindFirst) {
	// TODO: remove

	//if (bindFirst) {
	//	bind();
	//	Application::getInstance()->getDXManager()->getDeviceContext()->IASetInputLayout(m_inputLayout);
	//}

	//if (m_clippingPlaneHasChanged) {
	//	// Convert the clipping plane into view space
	//	updateWorldDataBuffer(Vector4::Transform(m_clippingPlane, (m_mView * m_mProjection).Transpose().Invert()));
	//	//updateWorldDataBuffer(m_clippingPlane);
	//	m_clippingPlaneHasChanged = false;
	//}

	//// Update the model data to match this model
	//updateModelDataBuffer(*model.getMaterial(), m_mView, m_mProjection);

	//auto devCon = Application::getInstance()->getDXManager()->getDeviceContext();

	//// Bind the texture if it exists
	//UINT numTextures;
	//auto tex = model.getMaterial()->getTextures(numTextures);
	//if (tex)
	//	devCon->PSSetShaderResources(0, numTextures, tex);

	//// Bind vertex 

	//// Bind vertex and instance buffers
	//UINT strides[2] = { sizeof(DynBlockDeferredInstancedGeometryShader::Vertex), sizeof(DynBlockDeferredInstancedGeometryShader::InstanceData) };
	//UINT offsets[2] = { 0, 0 };
	//ID3D11Buffer* bufferPtrs[2] = { *model.getVertexBuffer(), model.getInstanceBuffer() };
	//devCon->IASetVertexBuffers(0, 2, bufferPtrs, strides, offsets);

	//// Bind index buffer if one exitsts
	//auto iBuffer = model.getIndexBuffer();
	//if (iBuffer)
	//	devCon->IASetIndexBuffer(iBuffer, DXGI_FORMAT_R32_UINT, 0);

	//// Set topology
	//devCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//// Draw
	//if (iBuffer)
	//	devCon->DrawIndexedInstanced(model.getNumIndices(), model.getNumInstances(), 0, 0, 0);
	//else
	//	devCon->DrawInstanced(model.getNumVertices(), model.getNumInstances(), 0, 0);

	//ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	//Application::getInstance()->getDXManager()->getDeviceContext()->PSSetShaderResources(0, 1, nullSRV);

	//Application::getInstance()->getDXManager()->getDeviceContext()->GSSetShader(nullptr, 0, 0);
	//Application::getInstance()->getDXManager()->getDeviceContext()->PSSetShader(nullptr, 0, 0);
	//Application::getInstance()->getDXManager()->getDeviceContext()->VSSetShader(nullptr, 0, 0);

}
