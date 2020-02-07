#include "pch.h"
#include "ResourceManager.h"
#include "Sail/graphics/shader/Shader.h"
#include "Sail/api/shader/ShaderPipeline.h"

ResourceManager::ResourceManager() {
}
ResourceManager::~ResourceManager() {
	for (auto it : m_shaderSets) {
		delete it.second;
	}
}

//
// TextureData
//

void ResourceManager::loadTextureData(const std::string& filename, bool useAbsolutePath) {
	m_textureDatas.insert({ filename, std::make_unique<TextureData>(filename, useAbsolutePath) });
}
TextureData& ResourceManager::getTextureData(const std::string& filename) {
	auto pos = m_textureDatas.find(filename);
	if (pos == m_textureDatas.end())
		Logger::Error("Tried to access a resource that was not loaded. (" + filename + ") \n Use Application::getInstance()->getResourceManager().LoadTextureData(\"filename\") before accessing it.");

	return *pos->second;
}
bool ResourceManager::hasTextureData(const std::string& filename) {
	return m_textureDatas.find(filename) != m_textureDatas.end();
}

bool ResourceManager::releaseTextureData(const std::string& filename) {
	return m_textureDatas.erase(filename);
}

//
// DXTexture
//

void ResourceManager::loadTexture(const std::string& filename, bool useAbsolutePath) {
	SAIL_PROFILE_FUNCTION();

	if (!hasTexture(filename))
		m_textures.insert({ filename, std::unique_ptr<Texture>(Texture::Create(filename, useAbsolutePath)) });
}
Texture& ResourceManager::getTexture(const std::string& filename) {
	auto pos = m_textures.find(filename);
	if (pos == m_textures.end())
		Logger::Error("Tried to access a resource that was not loaded. (" + filename + ") \n Use Application::getInstance()->getResourceManager().loadTexture(\"" + filename + "\") before accessing it.");

	return *pos->second;
}
bool ResourceManager::hasTexture(const std::string& filename) {
	return m_textures.find(filename) != m_textures.end();
}


//
// Model
//

void ResourceManager::loadModel(const std::string& filename, Shader* shader, bool useAbsolutePath) {
	// Insert the new model
	m_fbxModels.insert({ filename, std::make_unique<ParsedScene>(filename, shader, useAbsolutePath) });
}
std::shared_ptr<Model> ResourceManager::getModel(const std::string& filename, Shader* shader, bool useAbsolutePath) {
	auto pos = m_fbxModels.find(filename);
	if (pos == m_fbxModels.end()) {
		// Model was not yet loaded, load it and return
		loadModel(filename, shader, useAbsolutePath);

		return m_fbxModels.find(filename)->second->getModel();
	}

	auto foundModel = pos->second->getModel();
	if (foundModel->getMesh(0)->getShader() != shader)
		Logger::Error("Tried to get model from resource manager that has already been loaded with a different shader!");

	return foundModel;
}
bool ResourceManager::hasModel(const std::string& filename) {
	return m_fbxModels.find(filename) != m_fbxModels.end();
}
