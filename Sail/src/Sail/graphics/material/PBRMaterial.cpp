#include "pch.h"
#include "PBRMaterial.h"
#include "Sail/api/shader/ShaderPipeline.h"
#include "Sail/graphics/shader/Shader.h"
#include "Sail/Application.h"

PBRMaterial::PBRMaterial()
	: Material(Material::PBR)
	, m_numTextures(3)
	, m_textures{ nullptr }
{
	m_pbrSettings.metalnessScale = 1.f;
	m_pbrSettings.roughnessScale = 1.f;
	m_pbrSettings.aoScale = 1.f;

	m_pbrSettings.hasAlbedoTexture = 0;
	m_pbrSettings.hasNormalTexture = 0;
	m_pbrSettings.hasMetalnessRoughnessAOTexture = 0;

	m_pbrSettings.modelColor = glm::vec4(1.0f);

	Application::getInstance()->getResourceManager().loadTexture("pbr/brdfLUT.tga");
	m_brdfLutTexture = &Application::getInstance()->getResourceManager().getTexture("pbr/brdfLUT.tga");
}
PBRMaterial::~PBRMaterial() {}

void PBRMaterial::bind(Shader* shader, void* cmdList) {
	ShaderPipeline* pipeline = shader->getPipeline();
	pipeline->trySetCBufferVar("sys_material", (void*)&getPBRSettings(), sizeof(PBRSettings));

	// TODO: check if this causes a problem in DX12
	// when a normal or specular texture is bound but not a diffuse one, the order will probably be wrong in dx12 shaders

	pipeline->setTexture("sys_texBrdfLUT", m_brdfLutTexture, cmdList);

	if (m_pbrSettings.hasAlbedoTexture) {
		pipeline->setTexture("sys_texAlbedo", m_textures[0], cmdList);
	}
	if (m_pbrSettings.hasNormalTexture) {
		pipeline->setTexture("sys_texNormal", m_textures[1], cmdList);
	}
	if (m_pbrSettings.hasMetalnessRoughnessAOTexture) {
		pipeline->setTexture("sys_texMRAO", m_textures[2], cmdList);
	}
}

void PBRMaterial::setMetalnessScale(float metalness) {
	m_pbrSettings.metalnessScale = metalness;
}

void PBRMaterial::setRoughnessScale(float roughness) {
	m_pbrSettings.roughnessScale = roughness;
}

void PBRMaterial::setAOScale(float ao) {
	m_pbrSettings.aoScale = ao;
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