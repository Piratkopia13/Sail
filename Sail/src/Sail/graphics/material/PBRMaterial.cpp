#include "pch.h"
#include "PBRMaterial.h"
#include "Sail/api/shader/PipelineStateObject.h"
#include "Sail/api/shader/Shader.h"
#include "Sail/Application.h"
#include "Sail/api/Texture.h"
#include "../Environment.h"

// TODO: remove this when textures can be bound from name in dx12
#ifdef _SAIL_DX12
#include "API/DX12/DX12API.h"
#include "API/DX12/resources/DescriptorHeap.h"
#endif

PBRMaterial::PBRMaterial()
	: Material(Material::PBR)
	, m_numTextures(6)
	, m_hasTransparency(false)
{
#if ALLOW_SEPARATE_MRAO
	m_numTextures += 3;
	m_pbrSettings.metalnessTexIndex = -1;
	m_pbrSettings.roughnessTexIndex = -1;
	m_pbrSettings.aoTexIndex = -1;
#endif
	textures.resize(m_numTextures);

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

void PBRMaterial::setEnvironment(const Environment& environment) {
	textures[3] = environment.getRadianceTexture();
	textures[4] = environment.getIrradianceTexture();
}

void PBRMaterial::setTextureIndex(unsigned int textureID, int index) {
	if (textureID == 0) m_pbrSettings.albedoTexIndex = index;
	else if (textureID == 1) m_pbrSettings.normalTexIndex = index;
	else if (textureID == 2) m_pbrSettings.mraoTexIndex = index;
	else if (textureID == 3) m_pbrSettings.radianceMapTexIndex = index;
	else if (textureID == 4) m_pbrSettings.irradianceMapTexIndex = index;
	else if (textureID == 5) m_pbrSettings.brdfLutTexIndex = index;
	else if (textureID == 6) m_pbrSettings.metalnessTexIndex = index;
	else if (textureID == 7) m_pbrSettings.roughnessTexIndex = index;
	else if (textureID == 8) m_pbrSettings.aoTexIndex = index;
}

void* PBRMaterial::getData() {
	return static_cast<void*>(&getPBRSettings());
}

unsigned int PBRMaterial::getDataSize() const {
	return sizeof(ShaderShared::PBRMaterial);
}

Shader* PBRMaterial::getShader(Renderer::Type rendererType) const {
	auto& resman = Application::getInstance()->getResourceManager();
	switch (rendererType) {
	case Renderer::FORWARD:
		return &resman.getShaderSet(Shaders::PBRMaterialShader);
		break;
	case Renderer::DEFERRED:
		if (m_hasTransparency || m_pbrSettings.modelColor.a < 1.0f) return nullptr; // Deferred pass can not draw transparent meshes, use forward pass instead
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

void PBRMaterial::enableTransparency(bool enable) {
	m_hasTransparency = enable;
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

void PBRMaterial::setMetalnessTexture(const std::string& filename, bool useAbsolutePath) {
	textures[6] = loadTexture(filename, useAbsolutePath);
}

void PBRMaterial::setRoughnessTexture(const std::string& filename, bool useAbsolutePath /*= false*/) {
	textures[7] = loadTexture(filename, useAbsolutePath);
}

void PBRMaterial::setAoTexture(const std::string& filename, bool useAbsolutePath /*= false*/) {
	textures[8] = loadTexture(filename, useAbsolutePath);
}

Texture* PBRMaterial::getTexture(unsigned int id) const {
	return textures[id];
}

ShaderShared::PBRMaterial& PBRMaterial::getPBRSettings() {
	return m_pbrSettings;
}