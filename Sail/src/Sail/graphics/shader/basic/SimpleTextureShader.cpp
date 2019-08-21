#include "SimpleTextureShader.h"

using namespace DirectX::SimpleMath;

D3D11_INPUT_ELEMENT_DESC SimpleTextureShader::IED[2] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA, 0 },
};

SimpleTextureShader::SimpleTextureShader() {

	ModelDataBuffer defaultBuffer = { Vector4::One, Matrix::Identity, Matrix::Identity };
	m_transformBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&defaultBuffer, sizeof(ModelDataBuffer)));
	m_sampler = std::make_unique<ShaderComponent::Sampler>();
	ClippingPlaneBuffer defaultClippingBuffer = { Vector4(0.f, 0.f, 0.f, 0.f) };
	m_clippingPlaneBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&defaultClippingBuffer, sizeof(ClippingPlaneBuffer)));

	// Compile shader and add to this shaderSet
	auto vsBlob = compileShader(L"SimpleTextureShader.hlsl", "VSMain", "vs_5_0");
	setVertexShader(vsBlob);

	// Compile shader and add to this shaderSet
	auto psBlob = compileShader(L"SimpleTextureShader.hlsl", "PSMain", "ps_5_0");
	setPixelShader(psBlob);

	// Create the inputlayout
	Application::getInstance()->getDXManager()->getDevice()->CreateInputLayout(IED, 2, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);

	// Done with the blobs, release them
	Memory::safeRelease(vsBlob);
	Memory::safeRelease(psBlob);

}
SimpleTextureShader::~SimpleTextureShader() {
	Memory::safeRelease(m_inputLayout);
}

void SimpleTextureShader::updateCamera(Camera& cam) {
	m_vpMatrix = cam.getViewProjection();
}

void SimpleTextureShader::updateBuffer(const DirectX::SimpleMath::Vector4& color, const DirectX::SimpleMath::Matrix& w, const DirectX::SimpleMath::Matrix& vp) const {
	ModelDataBuffer data = { color, w.Transpose(), vp.Transpose() };
	m_transformBuffer->updateData(&data, sizeof(data));
}

void SimpleTextureShader::setClippingPlane(const DirectX::SimpleMath::Vector4& clippingPlane) {
	ClippingPlaneBuffer data = { clippingPlane };
	m_clippingPlaneBuffer->updateData(&data, sizeof(data));
}

void SimpleTextureShader::bind() {
	ShaderSet::bind();

	// Set input layout as active
	Application::getInstance()->getDXManager()->getDeviceContext()->IASetInputLayout(m_inputLayout);

	// Bind the transform constant buffer
	m_transformBuffer->bind(ShaderComponent::VS, 0);
	// Bind clipping plane buffer
	m_clippingPlaneBuffer->bind(ShaderComponent::VS, 1);
	// Bind sampler
	m_sampler->bind();

}

void SimpleTextureShader::createBufferFromModelData(ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, ID3D11Buffer** instanceBuffer, const void* data) {

	Model::Data modelData = *(Model::Data*)data;

	if (modelData.numVertices <= 0 || !modelData.positions)
		Logger::Error("numVertices or position data not set for model");

	if (!modelData.texCoords) {
		Logger::Warning("Texture coordinates not set for model that will render with a texture shader");
	}

	// Create the vertex array that this shader uses
	SimpleTextureShader::Vertex* vertices = new SimpleTextureShader::Vertex[modelData.numVertices];

	for (UINT i = 0; i < modelData.numVertices; i++) {
		vertices[i].position = modelData.positions[i];
		if (modelData.texCoords)
			vertices[i].texCoord = modelData.texCoords[i];
		else
			vertices[i].texCoord = Vector2(0.f, 0.f);
	}

	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));

	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = modelData.numVertices * sizeof(SimpleTextureShader::Vertex);
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

void SimpleTextureShader::draw(Model& model, bool bindFirst) {
	// TODO: remove

	//if (bindFirst) {
	//	// Bind the shaders
	//	// Bind the input layout
	//	// bind constant buffer
	//	bind();
	//}

	//// Update the world matrix to match this model
	//auto modelColor = model.getMaterial()->getColor();
	//updateBuffer(modelColor, model.getTransform().getMatrix(), m_vpMatrix);

	//// Bind the texture if it exists
	//UINT numTextures;
	//auto tex = model.getMaterial()->getTextures(numTextures);
	//if (tex)
	//	Application::getInstance()->getDXManager()->getDeviceContext()->PSSetShaderResources(0, numTextures, tex);

	//// Bind vertex buffer
	//UINT stride = sizeof(SimpleTextureShader::Vertex);
	//UINT offset = 0;
	//Application::getInstance()->getDXManager()->getDeviceContext()->IASetVertexBuffers(0, 1, model.getVertexBuffer(), &stride, &offset);

	//// Bind index buffer if one exists
	//auto* iBuffer = model.getIndexBuffer();
	//if (iBuffer)
	//	Application::getInstance()->getDXManager()->getDeviceContext()->IASetIndexBuffer(iBuffer, DXGI_FORMAT_R32_UINT, 0);

	//// Set topology
	//Application::getInstance()->getDXManager()->getDeviceContext()->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	//// Draw
	//if (iBuffer)
	//	Application::getInstance()->getDXManager()->getDeviceContext()->DrawIndexed(model.getNumIndices(), 0, 0);
	//else
	//	Application::getInstance()->getDXManager()->getDeviceContext()->Draw(model.getNumVertices(), 0);

	//ID3D11ShaderResourceView* nullSRV[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
	//Application::getInstance()->getDXManager()->getDeviceContext()->PSSetShaderResources(0, 5, nullSRV);

}