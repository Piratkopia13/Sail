#include "pch.h"
#include "PhongMaterial.h"
#include "Sail/api/shader/ShaderPipeline.h"
#include "Sail/graphics/shader/Shader.h"
#include "Sail/Application.h"

PhongMaterial::PhongMaterial()
	: Material(Material::PHONG)
	, m_numTextures(3)
	, m_textures {nullptr}
{
	m_phongSettings.ka = 1.f;
	m_phongSettings.kd = 1.f;
	m_phongSettings.ks = 1.f;
	m_phongSettings.shininess = 10.f;
	m_phongSettings.modelColor = glm::vec4(1.0f);
	m_phongSettings.hasDiffuseTexture = 0;
	m_phongSettings.hasNormalTexture = 0;
	m_phongSettings.hasSpecularTexture = 0;

}
PhongMaterial::~PhongMaterial() { }

void PhongMaterial::bind(Shader* shader, void* cmdList) {
	ShaderPipeline* pipeline = shader->getPipeline();
	pipeline->trySetCBufferVar("sys_material", (void*)&getPhongSettings(), sizeof(PhongSettings));

	// TODO: check if this causes a problem in DX12
	// when a normal or specular texture is bound but not a diffuse one, the order will probably be wrong in dx12 shaders
	if (m_phongSettings.hasDiffuseTexture)
		pipeline->setTexture("sys_texDiffuse", m_textures[0], cmdList);
	if (m_phongSettings.hasNormalTexture)
		pipeline->setTexture("sys_texNormal", m_textures[1], cmdList);
	if (m_phongSettings.hasSpecularTexture)
		pipeline->setTexture("sys_texSpecular", m_textures[2], cmdList);
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
	m_textures[0] = loadTexture(filename, useAbsolutePath);
	m_phongSettings.hasDiffuseTexture = (filename.empty()) ? 0 : 1;
}
void PhongMaterial::setDiffuseTextureFromHandle(Texture* srv) {
	m_textures[0] = srv;
	m_phongSettings.hasDiffuseTexture = 1;
}


void PhongMaterial::setNormalTexture(const std::string& filename, bool useAbsolutePath) {
	m_textures[1] = loadTexture(filename, useAbsolutePath);
	m_phongSettings.hasNormalTexture = (filename.empty()) ? 0 : 1;
}
void PhongMaterial::setNormalTextureFromHandle(Texture* srv) {
	m_textures[1] = srv;
	m_phongSettings.hasNormalTexture = 1;
}


void PhongMaterial::setSpecularTexture(const std::string& filename, bool useAbsolutePath) {
	m_textures[2] = loadTexture(filename, useAbsolutePath);
	m_phongSettings.hasSpecularTexture = (filename.empty()) ? 0 : 1;
}
void PhongMaterial::setSpecularTextureFromHandle(Texture* srv) {
	m_textures[2] = srv;
	m_phongSettings.hasSpecularTexture = 1;
}

Texture* PhongMaterial::getTexture(unsigned int id) const {
	return m_textures[id];
}

PhongMaterial::PhongSettings& PhongMaterial::getPhongSettings() {
	return m_phongSettings;
}