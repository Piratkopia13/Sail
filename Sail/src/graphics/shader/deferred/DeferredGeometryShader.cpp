#include "DeferredGeometryShader.h"

using namespace DirectX;
using namespace SimpleMath;

D3D11_INPUT_ELEMENT_DESC DeferredGeometryShader::IED[5] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

DeferredGeometryShader::DeferredGeometryShader()
	: m_clippingPlaneHasChanged(false)
{

	// Set up constant buffers
	ModelDataBuffer defaultModelData = { Matrix::Identity, Matrix::Identity, Vector4::One, 1.f, 1.f, 1.f, 10.f, 0, 0, 0 };
	m_modelDataBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&defaultModelData, sizeof(ModelDataBuffer)));
	WorldDataBuffer defaultworldData = { Vector4::Zero };
	m_worldDataBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&defaultworldData, sizeof(WorldDataBuffer)));

	// Set up sampler
	m_sampler = std::make_unique<ShaderComponent::Sampler>(D3D11_TEXTURE_ADDRESS_WRAP, D3D11_FILTER_MIN_MAG_MIP_POINT);

	// Compile VS
	auto vsBlob = compileShader(L"deferred/GeometryShader.hlsl", "VSMain", "vs_5_0");
	// Add the VS to this ShaderSet
	setVertexShader(vsBlob);

	// Compile GS
	auto gsBlob = compileShader(L"deferred/GeometryShader.hlsl", "GSMain", "gs_5_0");
	// Add the GS to this ShaderSet
	setGeometryShader(gsBlob);

	// Compile PS
	auto psBlob = compileShader(L"deferred/GeometryShader.hlsl", "PSMain", "ps_5_0");
	// Add the PS to this ShaderSet
	setPixelShader(psBlob);

	// Create the input layout
	Application::getInstance()->getDXManager()->getDevice()->CreateInputLayout(IED, 5, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);

	// Done with the blobs, release them
	Memory::safeRelease(vsBlob);
	Memory::safeRelease(psBlob);

}
DeferredGeometryShader::~DeferredGeometryShader() {
	Memory::safeRelease(m_inputLayout);
}

void DeferredGeometryShader::updateCamera(Camera& cam) {
	m_mView = cam.getViewMatrix();
	m_mProjection = cam.getProjMatrix();
}

void DeferredGeometryShader::setClippingPlane(const DirectX::SimpleMath::Vector4& clippingPlane) {
	m_clippingPlane = clippingPlane;
	m_clippingPlaneHasChanged = true;
}

void DeferredGeometryShader::updateWorldDataBuffer(const DirectX::SimpleMath::Vector4& clippingPlaneVS) const {
	WorldDataBuffer data = { clippingPlaneVS };
	m_worldDataBuffer->updateData(&data, sizeof(data));
}

void DeferredGeometryShader::updateModelDataBuffer(const Material& material, const DirectX::SimpleMath::Matrix& wv, const DirectX::SimpleMath::Matrix& p) const {

	const bool* texFlags = material.getTextureFlags();

	ModelDataBuffer data = {
		wv.Transpose(), p.Transpose() ,
		material.getColor(),
		material.getKa(),
		material.getKd(),
		material.getKs(),
		material.getShininess(),
		{ (int)texFlags[0], (int)texFlags[1], (int)texFlags[2] }
	};

	m_modelDataBuffer->updateData(&data, sizeof(data));
}

void DeferredGeometryShader::bind() {

	// Call parent to bind shaders
	ShaderSet::bind();

	// Set input layout as active
	Application::getInstance()->getDXManager()->getDeviceContext()->IASetInputLayout(m_inputLayout);

	// Bind cbuffers
	m_modelDataBuffer->bind(ShaderComponent::VS, 0);
	m_modelDataBuffer->bind(ShaderComponent::PS, 0);
	m_worldDataBuffer->bind(ShaderComponent::GS, 0);
	// Bind sampler
	m_sampler->bind();

}

void DeferredGeometryShader::createBufferFromModelData(ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, ID3D11Buffer** instanceBuffer, const void* data) {

	Model::Data modelData = *(Model::Data*)data;

	if (modelData.numVertices <= 0 || !modelData.positions)
		Logger::Error("numVertices or position data not set for model");

	if (!modelData.texCoords)
		Logger::Warning("Texture coordinates not set for model that will render with a texture shader");

	// Create the vertex array that this shader uses
	DeferredGeometryShader::Vertex* vertices = new DeferredGeometryShader::Vertex[modelData.numVertices];

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
	vbd.ByteWidth = modelData.numVertices * sizeof(DeferredGeometryShader::Vertex);
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

}

void DeferredGeometryShader::draw(Model& model, bool bindFirst) {

	// TODO: remove

	//if (bindFirst)
	//	bind();

	//if (m_clippingPlaneHasChanged) {
	//	// Convert the clipping plane into view space
	//	updateWorldDataBuffer(Vector4::Transform(m_clippingPlane, (m_mView * m_mProjection).Transpose().Invert()));
	//	//updateWorldDataBuffer(m_clippingPlane);
	//	m_clippingPlaneHasChanged = false;
	//}

	//// Update the model data to match this model
	//updateModelDataBuffer(*model.getMaterial(), model.getTransform().getMatrix() * m_mView, m_mProjection);

	//auto devCon = Application::getInstance()->getDXManager()->getDeviceContext();

	//// Bind the texture if it exists
	//UINT numTextures;
	//auto tex = model.getMaterial()->getTextures(numTextures);
	//if (tex)
	//	devCon->PSSetShaderResources(0, numTextures, tex);

	//// Bind vertex buffer
	//UINT stride = sizeof(DeferredGeometryShader::Vertex);
	//UINT offset = 0;
	//devCon->IASetVertexBuffers(0, 1, model.getVertexBuffer(), &stride, &offset);

	//// Bind index buffer if one exitsts
	//auto iBuffer = model.getIndexBuffer();
	//if (iBuffer)
	//	devCon->IASetIndexBuffer(iBuffer, DXGI_FORMAT_R32_UINT, 0);

	//// Set topology
	//devCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//// Draw
	//if (iBuffer)
	//	devCon->DrawIndexed(model.getNumIndices(), 0, 0);
	//else
	//	devCon->Draw(model.getNumVertices(), 0);

	//ID3D11ShaderResourceView* nullSRV[1] = { nullptr };
	//Application::getInstance()->getDXManager()->getDeviceContext()->PSSetShaderResources(0, 1, nullSRV);

	//Application::getInstance()->getDXManager()->getDeviceContext()->GSSetShader(nullptr, 0, 0);
	//Application::getInstance()->getDXManager()->getDeviceContext()->PSSetShader(nullptr, 0, 0);
	//Application::getInstance()->getDXManager()->getDeviceContext()->VSSetShader(nullptr, 0, 0);

}
