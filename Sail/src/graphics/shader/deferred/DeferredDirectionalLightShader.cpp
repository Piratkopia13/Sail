#include "DeferredDirectionalLightShader.h"
#include "../../renderer/DeferredRenderer.h"

using namespace DirectX;
using namespace SimpleMath;

D3D11_INPUT_ELEMENT_DESC DeferredDirectionalLightShader::IED[1] = {
	{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 }
};

DeferredDirectionalLightShader::DeferredDirectionalLightShader() {

	// Set up constant buffers
	ModelDataBuffer defaultModelData = { Matrix::Identity };
	m_modelDataBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&defaultModelData, sizeof(ModelDataBuffer)));
	LightDataBuffer defaultLightData = { Vector3::Zero, 0.f, Vector3::Zero };
	m_lightDataBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&defaultLightData, sizeof(LightDataBuffer)));
	ShadowLightBuffer defaultShadowLightData = { Matrix::Identity, Matrix::Identity, Matrix::Identity };
	m_shadowLightBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&defaultShadowLightData, sizeof(ShadowLightBuffer)));

	// Set up sampler for point sampling
	m_sampler = std::make_unique<ShaderComponent::Sampler>(D3D11_TEXTURE_ADDRESS_WRAP, D3D11_FILTER_MIN_MAG_MIP_POINT);
	m_shadowSampler = std::make_unique<ShaderComponent::Sampler>(D3D11_TEXTURE_ADDRESS_WRAP, D3D11_FILTER_MIN_MAG_MIP_POINT);

	// Compile VS
	auto vsBlob = compileShader(L"deferred/DirectionalLightShader.hlsl", "VSMain", "vs_5_0");
	// Add the VS to this ShaderSet
	setVertexShader(vsBlob);

	// Compile PS
	auto psBlob = compileShader(L"deferred/DirectionalLightShader.hlsl", "PSMain", "ps_5_0");
	// Add the PS to this ShaderSet
	setPixelShader(psBlob);

	// Create the input layout
	Application::getInstance()->getDXManager()->getDevice()->CreateInputLayout(IED, 1, vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), &m_inputLayout);

	// Done with the blobs, release them
	Memory::safeRelease(vsBlob);
	Memory::safeRelease(psBlob);

}
DeferredDirectionalLightShader::~DeferredDirectionalLightShader() {
	Memory::safeRelease(m_inputLayout);
	/*Memory::safeRelease(m_texArraySRV);
	Memory::safeRelease(m_texArray);*/
}

void DeferredDirectionalLightShader::updateCamera(Camera& cam) {
	m_mV = cam.getViewMatrix();
	m_mInvP = cam.getProjMatrix().Invert();
}

void DeferredDirectionalLightShader::updateModelDataBuffer(const Matrix& invP) const {

	ModelDataBuffer data = {
		invP.Transpose()
	};

	m_modelDataBuffer->updateData(&data, sizeof(data));
}

//void DeferredDirectionalLightShader::createTextureArray(UINT width, UINT height, ID3D11Texture2D** depthTextures) {
//
//	m_depthTextures = depthTextures;
//
//	D3D11_TEXTURE2D_DESC texElementDesc;
//	depthTextures[0]->GetDesc(&texElementDesc);
//
//	auto devCon = Application::getInstance()->getDXManager()->getDeviceContext();
//
//	// Create a texture2d object for the array
//	D3D11_TEXTURE2D_DESC texArrayDesc;
//	ZeroMemory(&texArrayDesc, sizeof(texArrayDesc));
//	texArrayDesc.Width = texElementDesc.Width;
//	texArrayDesc.Height = texElementDesc.Height;
//	texArrayDesc.MipLevels = texElementDesc.MipLevels;
//	texArrayDesc.ArraySize = 2;
//	texArrayDesc.Format = texElementDesc.Format;
//	texArrayDesc.SampleDesc.Count = 1;
//	texArrayDesc.Usage = D3D11_USAGE_DEFAULT;
//	texArrayDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
//	texArrayDesc.CPUAccessFlags = 0;
//
//	// Create the texture2D
//	//ID3D11Texture2D* tex;
//	Application::getInstance()->getDXManager()->getDevice()->CreateTexture2D(&texArrayDesc, nullptr, &m_texArray);
//
//	// Copy individial texture elements into the texture array
//
//	for (UINT i = 0; i < DeferredRenderer::NUM_GBUFFERS - 1; i++) {
//		//D3D11_MAPPED_SUBRESOURCE mappedTex2D;
//		//devCon->Map(gbufferTextures[i], 0, D3D11_MAP_READ, 0, &mappedTex2D);
//		//devCon->UpdateSubresource(tex, D3D11CalcSubresource(0, i, 0), nullptr, mappedTex2D.pData, mappedTex2D.RowPitch, 0);
//		devCon->CopySubresourceRegion(m_texArray, D3D11CalcSubresource(0, i, 1), 0, 0, 0, depthTextures[i], D3D11CalcSubresource(0, 0, 1), nullptr);
//		//devCon->Unmap(gbufferTextures[i], 0);
//	}
//
//	// Create the shader resource view for the texture array
//	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
//	ZeroMemory(&srvDesc, sizeof(srvDesc));
//	srvDesc.Format = texArrayDesc.Format;
//	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2DARRAY;
//	srvDesc.Texture2DArray.MostDetailedMip = 0;
//	srvDesc.Texture2DArray.MipLevels = 1;
//	srvDesc.Texture2DArray.FirstArraySlice = 0;
//	srvDesc.Texture2DArray.ArraySize = DeferredRenderer::NUM_GBUFFERS - 1;
//
//	// Release the old SRV
//	//Memory::safeRelease(m_texArraySRV);
//	// Create the ShaderResourceView
//	Application::getInstance()->getDXManager()->getDevice()->CreateShaderResourceView(m_texArray, &srvDesc, &m_texArraySRV);
//
//	// Release the texture - since the SRV is bound to it it will keep in memory until the SRV is released
//	//Memory::safeRelease(tex);
//
//}

void DeferredDirectionalLightShader::setLight(const DirectionalLight& dl) {
	LightDataBuffer data;
	// Transform the vector into view space using the cameras view matrix
	// Using TransformNormal since this is a direction, translations should not be applied
	data.direction = Vector3::TransformNormal(dl.direction, m_mV);
	data.color = dl.color;
	m_lightDataBuffer->updateData(&data, sizeof(data));
}

void DeferredDirectionalLightShader::updateCameraBuffer(Camera& cam, Camera& dlCam) {
	ShadowLightBuffer data;

	// The inverted view matrix to transform from the player camera's view space to world space
	data.mInvV = cam.getViewMatrix().Transpose().Invert();
	data.mLightV = dlCam.getViewMatrix().Transpose();
	data.mLightP = dlCam.getProjMatrix().Transpose();

	m_shadowLightBuffer->updateData(&data, sizeof(data));
}

void DeferredDirectionalLightShader::bind() {

	// Call parent to bind shaders
	ShaderSet::bind();

	// Set input layout as active
	Application::getInstance()->getDXManager()->getDeviceContext()->IASetInputLayout(m_inputLayout);

	// Bind cbuffers
	m_modelDataBuffer->bind(ShaderComponent::VS, 0);
	m_lightDataBuffer->bind(ShaderComponent::PS, 0);
	m_shadowLightBuffer->bind(ShaderComponent::PS, 1);

	// Bind sampler
	m_sampler->bind();
	m_shadowSampler->bind(ShaderComponent::PS, 1);

}

void DeferredDirectionalLightShader::createBufferFromModelData(ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, ID3D11Buffer** instanceBuffer, const void* data) {

	Model::Data modelData = *(Model::Data*)data;

	if (modelData.numVertices <= 0 || !modelData.positions)
		Logger::Error("numVertices or position data not set for model");

	if (!modelData.texCoords)
		Logger::Warning("Texture coordinates not set for model that will render with a texture shader");

	// Create the vertex array that this shader uses
	DeferredDirectionalLightShader::Vertex* vertices = new DeferredDirectionalLightShader::Vertex[modelData.numVertices];

	for (UINT i = 0; i < modelData.numVertices; i++) {
		// Position
		vertices[i].position = modelData.positions[i];
	}

	D3D11_BUFFER_DESC vbd;
	ZeroMemory(&vbd, sizeof(vbd));

	vbd.Usage = D3D11_USAGE_IMMUTABLE;
	vbd.ByteWidth = modelData.numVertices * sizeof(DeferredDirectionalLightShader::Vertex);
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

void DeferredDirectionalLightShader::draw(Model& model, bool bindFirst) {

	// TODO: remove

	//if (bindFirst)
	//	bind();

	//// Update the model data to match this model
	//updateModelDataBuffer(m_mInvP);

	//auto devCon = Application::getInstance()->getDXManager()->getDeviceContext();

	//// Bind the texture if it exists
	//UINT numTextures;
	//auto tex = model.getMaterial()->getTextures(numTextures);

	//if (tex)
	//	devCon->PSSetShaderResources(0, 4, tex);

	//// Bind vertex buffer
	//UINT stride = sizeof(DeferredDirectionalLightShader::Vertex);
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

	//ID3D11ShaderResourceView* nullSRV[5] = { nullptr, nullptr, nullptr, nullptr, nullptr };
	//Application::getInstance()->getDXManager()->getDeviceContext()->PSSetShaderResources(0, 5, nullSRV);

}
