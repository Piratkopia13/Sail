#include "pch.h"
#include "PhongMaterial.h"
#include "Sail/api/shader/PipelineStateObject.h"
#include "Sail/api/shader/Shader.h"
#include "Sail/Application.h"

PhongMaterial::PhongMaterial()
	: Material(Material::PHONG)
	, m_numTextures(3)
{
	textures.resize(m_numTextures);

	m_phongSettings.ka = 1.f;
	m_phongSettings.kd = 1.f;
	m_phongSettings.ks = 1.f;
	m_phongSettings.shininess = 10.f;
	m_phongSettings.modelColor = glm::vec4(1.0f);
	m_phongSettings.diffuseTexIndex = -1;
	m_phongSettings.normalTexIndex = -1;
	m_phongSettings.specularTexIndex = -1;

}
PhongMaterial::~PhongMaterial() { }

void PhongMaterial::setTextureIndex(unsigned int textureID, int index) {
	if (textureID == 0) m_phongSettings.diffuseTexIndex = index;
	else if (textureID == 1) m_phongSettings.normalTexIndex = index;
	else if (textureID == 2) m_phongSettings.specularTexIndex = index;
}

void* PhongMaterial::getData() {
	return static_cast<void*>(&getPhongSettings());
}

unsigned int PhongMaterial::getDataSize() const {
	return sizeof(ShaderShared::PhongMaterial);
}

Shader* PhongMaterial::getShader(Renderer::Type rendererType) const {
	auto& resman = Application::getInstance()->getResourceManager();
	switch (rendererType) {
	case Renderer::FORWARD:
		return &resman.getShaderSet(Shaders::PhongMaterialShader);
		break;
	default:
		return nullptr;
		break;
	}
}

void PhongMaterial::setKa(float ka) {
	m_phongSettings.ka = ka;
}
void PhongMaterial::setKd(float kd) {
	m_phongSettings.kd = kd;
}
void PhongMaterial::setKs(float ks) {
	m_phongSettings.ks = ks;
}
void PhongMaterial::setShininess(float shininess) {
	m_phongSettings.shininess = shininess;
}
void PhongMaterial::setColor(const glm::vec4& color) {
	m_phongSettings.modelColor = color;
}


void PhongMaterial::setDiffuseTexture(const std::string& filename, bool useAbsolutePath) {
	textures[0] = loadTexture(filename, useAbsolutePath);
}
void PhongMaterial::setDiffuseTextureFromHandle(Texture* srv) {
	textures[0] = srv;
}


void PhongMaterial::setNormalTexture(const std::string& filename, bool useAbsolutePath) {
	textures[1] = loadTexture(filename, useAbsolutePath);
}
void PhongMaterial::setNormalTextureFromHandle(Texture* srv) {
	textures[1] = srv;
}


void PhongMaterial::setSpecularTexture(const std::string& filename, bool useAbsolutePath) {
	textures[2] = loadTexture(filename, useAbsolutePath);
}
void PhongMaterial::setSpecularTextureFromHandle(Texture* srv) {
	textures[2] = srv;
}

Texture* PhongMaterial::getTexture(unsigned int id) const {
	return textures[id];
}

ShaderShared::PhongMaterial& PhongMaterial::getPhongSettings() {
	return m_phongSettings;
}