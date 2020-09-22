#include "pch.h"
#include "PBRMaterial.h"
#include "Sail/api/shader/PipelineStateObject.h"
#include "Sail/api/shader/Shader.h"
#include "Sail/Application.h"
#include "../Environment.h"

// TODO: remove this when textures can be bound from name in dx12
#ifdef _SAIL_DX12
#include "API/DX12/DX12API.h"
#include "API/DX12/resources/DescriptorHeap.h"
#endif

PBRMaterial::PBRMaterial()
	: Material(Material::PBR)
	, m_numTextures(6)
{
	textures.resize(6);

	m_pbrSettings.metalnessScale = 0.f;
	m_pbrSettings.roughnessScale = 1.f;
	m_pbrSettings.aoIntensity = 0.f;

	m_pbrSettings.albedoTexIndex = -1;
	m_pbrSettings.normalTexIndex = -1;
	m_pbrSettings.mraoTexIndex = -1;
	
	m_pbrSettings.radianceMapTexIndex = -1;
	m_pbrSettings.irradianceMapTexIndex = -1;
	m_pbrSettings.brdfLutTexIndex = -1;

	m_pbrSettings.modelColor = glm::vec4(1.0f);

	Application::getInstance()->getResourceManager().loadTexture("pbr/brdfLUT.tga");
	m_brdfLutTexture = &Application::getInstance()->getResourceManager().getTexture("pbr/brdfLUT.tga");
	textures[5] = m_brdfLutTexture;
}
PBRMaterial::~PBRMaterial() {}

void PBRMaterial::bind(Shader* shader, Environment* environment, void* cmdList) {
	shader->trySetCBufferVar("sys_material", (void*)&getPBRSettings(), sizeof(PBRSettings), cmdList);

	// TODO: check if this causes a problem in DX12
	// when a normal or specular texture is bound but not a diffuse one, the order will probably be wrong in dx12 shaders

	shader->setTexture("sys_texBrdfLUT", m_brdfLutTexture, cmdList);

	if (environment) {
		shader->setTexture("irradianceMap", environment->getIrradianceTexture(), cmdList);
		shader->setTexture("radianceMap", environment->getRadianceTexture(), cmdList);
	} else {
		// Bind null srvs to fill out t1 and t2
		// TODO: change how textures are bound in dx12 so this code doesn't have to be here
#ifdef _SAIL_DX12
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_R16G16B16A16_FLOAT;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
		auto* api = Application::getInstance()->getAPI<DX12API>();
		for (unsigned int i = 0; i < 2; i++) {
			api->getDevice()->CreateShaderResourceView(nullptr, &srvDesc, api->getMainGPUDescriptorHeap()->getNextCPUDescriptorHandle());
		}
#endif
	}

	shader->setTexture("sys_texAlbedo", textures[0], cmdList);
	shader->setTexture("sys_texNormal", textures[1], cmdList);
	shader->setTexture("sys_texMRAO", textures[2], cmdList);
}

void PBRMaterial::setEnvironment(Environment* environment) {
	textures[3] = environment->getRadianceTexture();
	textures[4] = environment->getIrradianceTexture();
}

void PBRMaterial::setTextureIndex(unsigned int textureID, int index) {
	if (textureID == 0) m_pbrSettings.albedoTexIndex = index;
	else if (textureID == 1) m_pbrSettings.normalTexIndex = index;
	else if (textureID == 2) m_pbrSettings.mraoTexIndex = index;
	else if (textureID == 3) m_pbrSettings.radianceMapTexIndex = index;
	else if (textureID == 4) m_pbrSettings.irradianceMapTexIndex = index;
	else if (textureID == 5) m_pbrSettings.brdfLutTexIndex = index;
}

void* PBRMaterial::getData() {
	return static_cast<void*>(&getPBRSettings());
}

unsigned int PBRMaterial::getDataSize() const {
	return sizeof(PBRSettings);
}

Shader* PBRMaterial::getShader(Renderer::Type rendererType) const {
	auto& resman = Application::getInstance()->getResourceManager();
	switch (rendererType) {
	case Renderer::FORWARD:
		return &resman.getShaderSet(Shaders::PBRMaterialShader);
		break;
	case Renderer::DEFERRED:
		return &resman.getShaderSet(Shaders::DeferredGeometryPassShader);
		break;
	default:
		return nullptr;
		break;
	}
}

void PBRMaterial::setMetalnessScale(float metalness) {
	m_pbrSettings.metalnessScale = metalness;
}

void PBRMaterial::setRoughnessScale(float roughness) {
	m_pbrSettings.roughnessScale = roughness;
}

void PBRMaterial::setAOIntensity(float intensity) {
	m_pbrSettings.aoIntensity = intensity;
}

void PBRMaterial::setColor(const glm::vec4& color) {
	m_pbrSettings.modelColor = color;
}

void PBRMaterial::setAlbedoTexture(const std::string& filename, bool useAbsolutePath) {
	textures[0] = loadTexture(filename, useAbsolutePath);
}

void PBRMaterial::setNormalTexture(const std::string& filename, bool useAbsolutePath) {
	textures[1] = loadTexture(filename, useAbsolutePath);
}

void PBRMaterial::setMetalnessRoughnessAOTexture(const std::string& filename, bool useAbsolutePath) {
	textures[2] = loadTexture(filename, useAbsolutePath);
}

Texture* PBRMaterial::getTexture(unsigned int id) const {
	return textures[id];
}

PBRMaterial::PBRSettings& PBRMaterial::getPBRSettings() {
	return m_pbrSettings;
}