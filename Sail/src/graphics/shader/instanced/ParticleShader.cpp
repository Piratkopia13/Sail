#include "ParticleShader.h"

using namespace DirectX;
using namespace SimpleMath;

D3D11_INPUT_ELEMENT_DESC ParticleShader::IED[5] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "TEXCOORD", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "TEXCOORD", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
	{ "TEXCOORD", 3, DXGI_FORMAT_R32_FLOAT, 1, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_INSTANCE_DATA, 1 },
};

ParticleShader::ParticleShader() {

	// Set up constant buffers
	CameraBuffer defaultCamData = { Matrix::Identity, Vector3::Zero };
	m_cameraDataBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&defaultCamData, sizeof(CameraBuffer)));
	SpriteData defaultSpriteData = { 3, 1.f };
	m_spriteDataBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&defaultSpriteData, sizeof(SpriteData)));

	// Set up sampler for point sampling
	m_sampler = std::make_unique<ShaderComponent::Sampler>(D3D11_TEXTURE_ADDRESS_CLAMP, D3D11_FILTER_MIN_MAG_MIP_LINEAR);

	// Compile VS
	auto vsBlob = compileShader(L"instanced/ParticleShader.hlsl", "VSMain", "vs_5_0");
	// Add the VS to this ShaderSet
	setVertexShader(vsBlob);

	// Compile GS
	auto gsBlob = compileShader(L"instanced/ParticleShader.hlsl", "GSMain", "gs_5_0");
	// Add the GS to this ShaderSet
	setGeometryShader(gsBlob);

	// Compile PS
	auto psBlob = compileShader(L"instanced/ParticleShader.hlsl", "PSMain", "ps_5_0");
	// Add the PS to this ShaderSet
	setPixelShader(psBlob);

	// Create the input layout
	Application::getInstance()->getDXManager()->getDevice()->CreateInputLayout(IED, 5, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);

	// Done with the blobs, release them
	Memory::safeRelease(vsBlob);
	Memory::safeRelease(psBlob);

}
ParticleShader::~ParticleShader() {
	Memory::safeRelease(m_inputLayout);
}

void ParticleShader::bind() {
	// Call parent to bind shaders
	ShaderSet::bind();

	// Set input layout as active
	Application::getInstance()->getDXManager()->getDeviceContext()->IASetInputLayout(m_inputLayout);

	// Bind cbuffers
	m_cameraDataBuffer->bind(ShaderComponent::GS, 0);
	m_spriteDataBuffer->bind(ShaderComponent::GS, 1);

	// Bind sampler
	m_sampler->bind();
}

void ParticleShader::createBufferFromModelData(ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, ID3D11Buffer** instanceBuffer, const void* data) {
	Model::Data modelData = *(Model::Data*)data;

	if (modelData.numVertices <= 0 || !modelData.positions)
		Logger::Error("numVertices or position data not set for model");

	// Create the vertex array that this shader uses
	ParticleShader::Vertex* vertices = new ParticleShader::Vertex[modelData.numVertices];

	for (UINT i = 0; i < modelData.numVertices; i++) {
		// Position
		vertices[i].position = modelData.positions[i];
	}

	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));

	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = modelData.numVertices * sizeof(ParticleShader::Vertex);
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

void ParticleShader::draw(Model& model, bool bindFirst, UINT instanceCount) {
	// TODO: remove

	//if (bindFirst)
	//	bind();

	//// Update the model data to match this model
	//updateCameraBuffer(m_mV * m_mP);

	//auto devCon = Application::getInstance()->getDXManager()->getDeviceContext();

	//// Bind the texture if it exists
	//UINT numTextures;
	//auto tex = model.getMaterial()->getTextures(numTextures);
	//if (tex)
	//	devCon->PSSetShaderResources(0, numTextures, tex);

	//// Bind vertex and instance buffers
	//UINT strides[2] = { sizeof(ParticleShader::Vertex), sizeof(InstanceData) };
	//UINT offsets[2] = { 0, 0 };
	//ID3D11Buffer* bufferPtrs[2] = { *model.getVertexBuffer(), model.getInstanceBuffer() };
	//devCon->IASetVertexBuffers(0, 2, bufferPtrs, strides, offsets);

	//// Bind index buffer if one exists
	//auto iBuffer = model.getIndexBuffer();
	//if (iBuffer)
	//	devCon->IASetIndexBuffer(iBuffer, DXGI_FORMAT_R32_UINT, 0);

	//// Set topology
	//devCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_POINTLIST);

	//UINT instances = (instanceCount == -1) ? model.getNumInstances() : instanceCount;

	//// Draw
	//if (iBuffer)
	//	devCon->DrawIndexedInstanced(model.getNumIndices(), instances, 0, 0, 0);
	//else
	//	devCon->DrawInstanced(model.getNumVertices(), instances, 0, 0);

	//ID3D11ShaderResourceView* nullSRV[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
	//Application::getInstance()->getDXManager()->getDeviceContext()->PSSetShaderResources(0, 5, nullSRV);
	//Application::getInstance()->getDXManager()->getDeviceContext()->GSSetShader(nullptr, 0, 0);
}

void ParticleShader::updateCamera(Camera& cam) {
	m_mV = cam.getViewMatrix();
	m_mP = cam.getProjMatrix();
	m_camPos = cam.getPosition();
}

void ParticleShader::updateSpriteData(UINT spritesPerRow, float scale) {
	SpriteData data = {
		spritesPerRow,
		scale
	};
	m_spriteDataBuffer->updateData(&data, sizeof(data));
}

void ParticleShader::updateInstanceData(const void* instanceData, UINT bufferSize, ID3D11Buffer* instanceBuffer) {

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	Application::getInstance()->getDXManager()->getDeviceContext()->Map(instanceBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	std::memcpy(mappedResource.pData, instanceData, bufferSize);
	Application::getInstance()->getDXManager()->getDeviceContext()->Unmap(instanceBuffer, 0);

}

void ParticleShader::updateCameraBuffer(const DirectX::SimpleMath::Matrix& vp) const {
	CameraBuffer data = {
		vp.Transpose(),
		m_camPos
	};
	m_cameraDataBuffer->updateData(&data, sizeof(data));
}
