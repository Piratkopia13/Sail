#include "pch.h"
#include "PhongMaterial.h"
#include "Sail/api/shader/ShaderPipeline.h"
#include "Sail/graphics/shader/Shader.h"
#include "Sail/Application.h"

PhongMaterial::PhongMaterial(Shader* shader)
	: m_numTextures(3)
	, m_shader(shader)
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

/*Not safe for multithreaded commandlist recording(d3d12)*/
void PhongMaterial::bind(void* cmdList) {
	ShaderPipeline* pipeline = m_shader->getPipeline();
	pipeline->trySetCBufferVar("sys_material", (void*)&getPhongSettings(), sizeof(PhongSettings));

	// TODO: check if this causes a problem in DX12
	// when a normal or specular texture is bound but not a diffuse one, the order will probably be wrong in dx12 shaders
	if (m_phongSettings.hasDiffuseTexture)
		pipeline->setTexture2D("sys_texDiffuse", m_textures[0], cmdList);
	if (m_phongSettings.hasNormalTexture)
		pipeline->setTexture2D("sys_texNormal", m_textures[1], cmdList);
	if (m_phongSettings.hasSpecularTexture)
		pipeline->setTexture2D("sys_texSpecular", m_textures[2], cmdList);
	//m_shader->bind();
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


void PhongMaterial::setDiffuseTexture(const std::string& filename) {
	getAndInsertTexture(filename, 0);
	m_phongSettings.hasDiffuseTexture = 1;
}
void PhongMaterial::setDiffuseTextureFromHandle(Texture* srv) {
	m_textures[0] = srv;
	m_phongSettings.hasDiffuseTexture = 1;
}


void PhongMaterial::setNormalTexture(const std::string& filename) {
	getAndInsertTexture(filename, 1);
	m_phongSettings.hasNormalTexture = 1;
}
void PhongMaterial::setNormalTextureFromHandle(Texture* srv) {
	m_textures[1] = srv;
	m_phongSettings.hasNormalTexture = 1;
}


void PhongMaterial::setSpecularTexture(const std::string& filename) {
	getAndInsertTexture(filename, 2);
	m_phongSettings.hasSpecularTexture = 1;
}
void PhongMaterial::setSpecularTextureFromHandle(Texture* srv) {
	m_textures[2] = srv;
	m_phongSettings.hasSpecularTexture = 1;
}


//void Material::setTextures(ID3D11ShaderResourceView** srvs, UINT numTextures) {
//	m_numTextures = numTextures;
//	m_customSRVs = srvs;
//}


void PhongMaterial::getAndInsertTexture(const std::string& filename, int arraySlot) {
	Texture* t = &Application::getInstance()->getResourceManager().getTexture(filename);
	m_textures[arraySlot] = t;
}

//ID3D11ShaderResourceView* const* Material::getTextures(UINT& numTextures) {
//	numTextures = m_numTextures;
//	if (m_customSRVs)
//		return m_customSRVs;
//	else
//		return m_srvs;
//}

Texture* PhongMaterial::getTexture(unsigned int id) const {
	return m_textures[id];
}

const PhongMaterial::PhongSettings& PhongMaterial::getPhongSettings() const {
	return m_phongSettings;
}

Shader* PhongMaterial::getShader() const {
	return m_shader;
}

// TODO: remove
//float Material::getKa() const {
//	return m_phongSettings.ka;
//}
//float Material::getKd() const {
//	return m_phongSettings.kd;
//}
//float Material::getKs() const {
//	return m_phongSettings.ks;
//}
//float Material::getShininess() const {
//	return m_phongSettings.shininess;
//}
//const glm::vec4& Material::getColor() const {
//	return m_phongSettings.modelColor;
//}
//const bool* Material::getTextureFlags() const {
//	return m_phongSettings.textureFlags;
//}