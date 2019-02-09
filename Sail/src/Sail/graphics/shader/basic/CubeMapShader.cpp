#include "CubeMapShader.h"

using namespace DirectX;
using namespace SimpleMath;

D3D11_INPUT_ELEMENT_DESC CubeMapShader::IED[1] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

CubeMapShader::CubeMapShader()
	: m_clippingPlaneHasChanged(false)
	, m_cameraPosHasChanged(false)
{

	// Set up constant buffers
	ModelDataBuffer defaultModelData = { Matrix::Identity, Matrix::Identity };
	m_modelDataBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&defaultModelData, sizeof(ModelDataBuffer)));
	WorldDataBuffer defaultworldData = { Vector4::Zero, Vector3::Zero };
	m_worldDataBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&defaultworldData, sizeof(WorldDataBuffer)));

	// Set up sampler
	m_sampler = std::make_unique<ShaderComponent::Sampler>();

	// Compile VS
	auto vsBlob = compileShader(L"CubeMapShader.hlsl", "VSMain", "vs_5_0");
	// Add the VS to this ShaderSet
	setVertexShader(vsBlob);

	// Compile PS
	auto psBlob = compileShader(L"CubeMapShader.hlsl", "PSMain", "ps_5_0");
	// Add the PS to this ShaderSet
	setPixelShader(psBlob);

	// Create the input layout
	Application::getInstance()->getDXManager()->getDevice()->CreateInputLayout(IED, 1, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);

	// Done with the blobs, release them
	Memory::safeRelease(vsBlob);
	Memory::safeRelease(psBlob);

}
CubeMapShader::~CubeMapShader() {
	Memory::safeRelease(m_inputLayout);
}

void CubeMapShader::updateCamera(Camera& cam) {
	m_vpMatrix = cam.getViewProjection();
	m_cameraPos = cam.getPosition();
	m_cameraPosHasChanged = true;
}

void CubeMapShader::setClippingPlane(const DirectX::SimpleMath::Vector4& clippingPlane) {
	m_clippingPlane = clippingPlane;
	m_clippingPlaneHasChanged = true;
}
void CubeMapShader::updateWorldDataBuffer() const {
	WorldDataBuffer data = { m_clippingPlane, m_cameraPos };
	m_worldDataBuffer->updateData(&data, sizeof(data));
}

void CubeMapShader::updateModelDataBuffer(const Material& material, const DirectX::SimpleMath::Matrix& w, const DirectX::SimpleMath::Matrix& vp) const {

	const bool* texFlags = material.getTextureFlags();

	ModelDataBuffer data = { w.Transpose(), vp.Transpose() };

	m_modelDataBuffer->updateData(&data, sizeof(data));
}

void CubeMapShader::bind() {

	// Call parent to bind shaders
	ShaderSet::bind();

	// Set input layout as active
	Application::getInstance()->getDXManager()->getDeviceContext()->IASetInputLayout(m_inputLayout);

	// Bind cbuffers
	m_modelDataBuffer->bind(ShaderComponent::VS, 0);
	m_worldDataBuffer->bind(ShaderComponent::VS, 1);
	// Bind sampler
	m_sampler->bind();

}

void CubeMapShader::createBufferFromModelData(ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, ID3D11Buffer** instanceBuffer, const void* data) {

	Model::Data modelData = *(Model::Data*)data;

	if (modelData.numVertices <= 0 || !modelData.positions)
		Logger::Error("numVertices or position data not set for model");

	// Create the vertex array that this shader uses
	CubeMapShader::Vertex* vertices = new CubeMapShader::Vertex[modelData.numVertices];

	for (UINT i = 0; i < modelData.numVertices; i++) {
		// Position
		vertices[i].position = modelData.positions[i];
	}

	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));

	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = modelData.numVertices * sizeof(CubeMapShader::Vertex);
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

void CubeMapShader::draw(Model& model, bool bindFirst) {

	// TODO: remove
	//if (bindFirst)
	//	bind();

	//if (m_cameraPosHasChanged || m_clippingPlaneHasChanged) {
	//	updateWorldDataBuffer();
	//	m_cameraPosHasChanged = false;
	//	m_clippingPlaneHasChanged = false;
	//}

	//// Update the model data to match this model
	//updateModelDataBuffer(*model.getMaterial(), model.getTransform().getMatrix(), m_vpMatrix);

	//auto devCon = Application::getInstance()->getDXManager()->getDeviceContext();

	//// Bind the texture if it exists
	//UINT numTextures;
	//auto tex = model.getMaterial()->getTextures(numTextures);
	//if (tex)
	//	devCon->PSSetShaderResources(0, numTextures, tex);

	//// Bind vertex buffer
	//UINT stride = sizeof(CubeMapShader::Vertex);
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

}
