#include "pch.h"
#include "MaterialShader.h"

MaterialShader::MaterialShader()
	: ShaderPipeline("MaterialShader.hlsl")
	, m_clippingPlaneHasChanged(false)
	//, m_cameraPosHasChanged(false)
{

	// Set up constant buffers
	/*ModelDataBuffer defaultModelData = { Matrix::Identity, Matrix::Identity, glm::vec4::One, 1.f, 1.f, 1.f, 10.f, 0, 0, 0 };
	m_modelDataBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&defaultModelData, sizeof(ModelDataBuffer)));
	WorldDataBuffer defaultworldData = { glm::vec4(0.f), glm::vec3(0.f) };
	m_worldDataBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&defaultworldData, sizeof(WorldDataBuffer)));
	LightsBuffer defaultLightsBuffer;
	m_lightsBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(new ShaderComponent::ConstantBuffer(&defaultLightsBuffer, sizeof(LightsBuffer)));*/

	// Set up sampler
	//m_sampler = std::make_unique<ShaderComponent::Sampler>();

	// Compile VS
	//auto vsBlob = compileShader(L"MaterialShader.hlsl", "VSMain", "vs_5_0");
	//// Add the VS to this ShaderSet
	//setVertexShader(vsBlob);

	//// Compile PS
	//auto psBlob = compileShader(L"MaterialShader.hlsl", "PSMain", "ps_5_0");
	//// Add the PS to this ShaderSet
	//setPixelShader(psBlob);

	// Create the input layout
	inputLayout->pushVec3(InputLayout::POSITION, "POSITION", 0);
	inputLayout->pushVec2(InputLayout::TEXCOORD, "TEXCOORD", 0);
	inputLayout->pushVec3(InputLayout::NORMAL, "NORMAL", 0);
	inputLayout->pushVec3(InputLayout::TANGENT, "TANGENT", 0);
	inputLayout->pushVec3(InputLayout::BITANGENT, "BINORMAL", 0);
	inputLayout->create(VSBlob);

	// Done with the blobs, release them
	/*Memory::safeRelease(vsBlob);
	Memory::safeRelease(psBlob);*/

}
MaterialShader::~MaterialShader() {
}

//void MaterialShader::updateCamera(Camera& cam) {
//	m_vpMatrix = cam.getViewProjection();
//}

void MaterialShader::setClippingPlane(const glm::vec4& clippingPlane) {
	m_clippingPlane = clippingPlane;
	m_clippingPlaneHasChanged = true;
}

//void MaterialShader::updateWorldDataBuffer() const {
//	WorldDataBuffer data = { m_clippingPlane, m_cameraPos };
//	m_worldDataBuffer->updateData(&data, sizeof(data));
//}

//void MaterialShader::updateLights(const Lights& lights) {
//	m_dirLightData.dirLight = lights.getDL();
//	auto& pLights = lights.getPLs();
//	// Copy the x first lights into the buffer
//	for (unsigned int i = 0; i < 8; i++) {
//		if (i >= pLights.size()) break;
//		m_pointLightsData.pLights[i].attenuation = pLights[i].getAttenuation().quadratic;
//		m_pointLightsData.pLights[i].color = pLights[i].getColor();
//		m_pointLightsData.pLights[i].position = pLights[i].getPosition();
//	}
//}

//void MaterialShader::updateModelDataBuffer(const Material& material, const glm::mat4& w, const glm::mat4& vp) const {
//
//	const bool* texFlags = material.getTextureFlags();
//
//	ModelDataBuffer data = {
//		w.Transpose(), vp.Transpose() ,
//		material.getColor(),
//		material.getKa(),
//		material.getKd(),
//		material.getKs(),
//		material.getShininess(),
//		{(int)texFlags[0], (int)texFlags[1], (int)texFlags[2] }
//	};
//
//	m_modelDataBuffer->updateData(&data, sizeof(data));
//}

void MaterialShader::bind() {

	// Call parent to bind shaders
	ShaderPipeline::bind();

	//// Set input layout as active
	//inputLayout.bind();

	// Bind cbuffers
	/*m_modelDataBuffer->bind(ShaderComponent::VS, 0);
	m_modelDataBuffer->bind(ShaderComponent::PS, 0);
	m_worldDataBuffer->bind(ShaderComponent::VS, 1);
	m_lightsBuffer->bind(ShaderComponent::VS, 2);*/
	// Bind sampler
	//m_sampler->bind();

	// TODO: move these to the renderer(?)
	/*setCBufferVar("sys_mVP", &m_vpMatrix.Transpose(), sizeof(m_vpMatrix));
	setCBufferVar("sys_cameraPos", &m_cameraPos, sizeof(m_cameraPos));*/
	/*setCBufferVar("dirLight", &m_dirLightData, sizeof(DirLightBuffer));
	setCBufferVar("pointLights", &m_pointLightsData, sizeof(PointLightsBuffer));*/

	// Update the model data to match this model
	//updateModelDataBuffer(*model.getMaterial(), model.getTransform().getMatrix(), m_vpMatrix); // TODO: fix

}

//void MaterialShader::createBufferFromModelData(ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, ID3D11Buffer** instanceBuffer, const void* data) {

	//Model::Data modelData = *(Model::Data*)data;

	//if (modelData.numVertices <= 0 || !modelData.positions)
	//	Logger::Error("numVertices or position data not set for model");

	//if (!modelData.texCoords)
	//	Logger::Warning("Texture coordinates not set for model that will render with a texture shader");

	//// Create the vertex array that this shader uses
	//MaterialShader::Vertex* vertices = new MaterialShader::Vertex[modelData.numVertices];

	//for (UINT i = 0; i < modelData.numVertices; i++) {
	//	// Position
	//	vertices[i].position = modelData.positions[i];
	//	// Tex coords
	//	if (modelData.texCoords)
	//		vertices[i].texCoords = modelData.texCoords[i];
	//	else
	//		vertices[i].texCoords = glm::vec2(0.f);
	//	// Normals
	//	if (modelData.normals)
	//		vertices[i].normal = modelData.normals[i];
	//	else
	//		vertices[i].normal = glm::vec3(0.f);
	//	// Tangents
	//	if (modelData.tangents)
	//		vertices[i].tangent = modelData.tangents[i];
	//	else
	//		vertices[i].tangent = glm::vec3(0.f);
	//	// Bitangents
	//	if (modelData.bitangents)
	//		vertices[i].bitangent = modelData.bitangents[i];
	//	else
	//		vertices[i].bitangent = glm::vec3(0.f);
	//}

	//D3D11_BUFFER_DESC vbd;
	//ZeroMemory(&vbd, sizeof(vbd));

	//vbd.Usage = D3D11_USAGE_IMMUTABLE;
	//vbd.ByteWidth = modelData.numVertices * sizeof(MaterialShader::Vertex);
	//vbd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	//vbd.CPUAccessFlags = 0;

	//D3D11_SUBRESOURCE_DATA vertexData;
	//ZeroMemory(&vertexData, sizeof(vertexData));
	//vertexData.pSysMem = vertices;

	//// Create the vertex buffer
	//ThrowIfFailed(Application::getInstance()->getAPI()->getDevice()->CreateBuffer(&vbd, &vertexData, vertexBuffer));
	//// Delete vertices from cpu memory
	//Memory::safeDeleteArr(vertices);

	//// Set up index buffer if indices are set
	//if (modelData.numIndices > 0) {

	//	ULONG* indices = new ULONG[modelData.numIndices];

	//	// Fill the array with the model indices
	//	for (UINT i = 0; i < modelData.numIndices; i++) {
	//		indices[i] = modelData.indices[i];
	//	}

	//	// Set up index buffer description
	//	D3D11_BUFFER_DESC ibd;
	//	ZeroMemory(&ibd, sizeof(ibd));
	//	ibd.Usage = D3D11_USAGE_IMMUTABLE;
	//	ibd.ByteWidth = modelData.numIndices * sizeof(UINT);
	//	ibd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	//	ibd.CPUAccessFlags = 0;

	//	D3D11_SUBRESOURCE_DATA indexData;
	//	ZeroMemory(&indexData, sizeof(indexData));
	//	indexData.pSysMem = indices;

	//	// Create the index buffer
	//	ThrowIfFailed(Application::getInstance()->getAPI()->getDevice()->CreateBuffer(&ibd, &indexData, indexBuffer));
	//	// Delete indices from cpu memory
	//	Memory::safeDeleteArr(indices);
	//}

//}

// TODO: remove
//void MaterialShader::draw(Model& model, bool bindFirst) {
//
//	if (bindFirst)
//		bind();
//
//	if (m_cameraPosHasChanged || m_clippingPlaneHasChanged) {
//		updateWorldDataBuffer();
//		m_cameraPosHasChanged = false;
//		m_clippingPlaneHasChanged = false;
//	}
//
//	// Update the model data to match this model
//	updateModelDataBuffer(*model.getMaterial(), model.getTransform().getMatrix(), m_vpMatrix);
//
//	auto devCon = Application::getInstance()->getAPI()->getDeviceContext();
//
//	// Bind the texture if it exists
//	UINT numTextures;
//	auto tex = model.getMaterial()->getTextures(numTextures);
//	if (tex)
//		devCon->PSSetShaderResources(0, numTextures, tex);
//
//	// Bind vertex buffer
//	UINT stride = sizeof(MaterialShader::Vertex);
//	UINT offset = 0;
//	devCon->IASetVertexBuffers(0, 1, model.getVertexBuffer(), &stride, &offset);
//
//	// Bind index buffer if one exitsts
//	auto iBuffer = model.getIndexBuffer();
//	if (iBuffer)
//		devCon->IASetIndexBuffer(iBuffer, DXGI_FORMAT_R32_UINT, 0);
//
//	// Set topology
//	devCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
//
//	// Draw
//	if (iBuffer)
//		devCon->DrawIndexed(model.getNumIndices(), 0, 0);
//	else
//		devCon->Draw(model.getNumVertices(), 0);
//
//}
