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
	, m_numTextures(3)
	, m_textures{ nullptr }
{
	m_pbrSettings.metalnessScale = 1.f;
	m_pbrSettings.roughnessScale = 1.f;
	m_pbrSettings.aoIntensity = 0.f;

	m_pbrSettings.hasAlbedoTexture = 0;
	m_pbrSettings.hasNormalTexture = 0;
	m_pbrSettings.hasMetalnessRoughnessAOTexture = 0;

	m_pbrSettings.modelColor = glm::vec4(1.0f);

	Application::getInstance()->getResourceManager().loadTexture("pbr/brdfLUT.tga");
	m_brdfLutTexture = &Application::getInstance()->getResourceManager().getTexture("pbr/brdfLUT.tga");
}
PBRMaterial::~PBRMaterial() {}

void PBRMaterial::bind(Shader* shader, Environment* environment, void* cmdList) {
	shader->trySetCBufferVar("sys_material", (void*)&getPBRSettings(), sizeof(PBRSettings));

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

	shader->setTexture("sys_texAlbedo", m_textures[0], cmdList);
	shader->setTexture("sys_texNormal", m_textures[1], cmdList);
	shader->setTexture("sys_texMRAO", m_textures[2], cmdList);
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
	m_textures[0] = loadTexture(filename, useAbsolutePath);
	m_pbrSettings.hasAlbedoTexture = (filename.empty()) ? 0 : 1;
}

void PBRMaterial::setNormalTexture(const std::string& filename, bool useAbsolutePath) {
	m_textures[1] = loadTexture(filename, useAbsolutePath);
	m_pbrSettings.hasNormalTexture = (filename.empty()) ? 0 : 1;
}

void PBRMaterial::setMetalnessRoughnessAOTexture(const std::string& filename, bool useAbsolutePath) {
	m_textures[2] = loadTexture(filename, useAbsolutePath);
	m_pbrSettings.hasMetalnessRoughnessAOTexture = (filename.empty()) ? 0 : 1;
}

Texture* PBRMaterial::getTexture(unsigned int id) const {
	return m_textures[id];
}

PBRMaterial::PBRSettings& PBRMaterial::getPBRSettings() {
	return m_pbrSettings;
}