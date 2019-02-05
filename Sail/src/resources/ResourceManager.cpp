#include "pch.h"
#include "ResourceManager.h"
#include "../graphics/shader/deferred/DeferredGeometryShader.h"
#include "audio/SoundManager.h"
#include "../graphics/shader/ShaderSet.h"

ResourceManager::ResourceManager() {
	m_soundManager = std::make_unique<SoundManager>();
}
ResourceManager::~ResourceManager() {
	for (auto it : m_shaderSets) {
		delete it.second;
	}
}

//
// TextureData
//

void ResourceManager::loadTextureData(const std::string& filename) {
	m_textureDatas.insert({ filename, std::make_unique<TextureData>(filename) });
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

//
// DXTexture
//

void ResourceManager::loadDXTexture(const std::string& filename) {

	m_dxTextures.insert({ filename, std::make_unique<DXTexture>(filename) });
}
DXTexture& ResourceManager::getDXTexture(const std::string& filename) {
	auto pos = m_dxTextures.find(filename);
	if (pos == m_dxTextures.end())
		Logger::Error("Tried to access a resource that was not loaded. (" + filename + ") \n Use Application::getInstance()->getResourceManager().LoadDXTexture(" + filename + ") before accessing it.");

	return *pos->second;
}
bool ResourceManager::hasDXTexture(const std::string& filename) {
	return m_dxTextures.find(filename) != m_dxTextures.end();
}


//
// Model
//

void ResourceManager::loadModel(const std::string& filename, ShaderSet* shaderSet) {
	// Insert the new model
	m_fbxModels.insert({ filename, std::make_unique<ParsedScene>(filename, shaderSet) });
}
Model& ResourceManager::getModel(const std::string& filename, ShaderSet* shaderSet) {
	auto pos = m_fbxModels.find(filename);
	if (pos == m_fbxModels.end()) {
		// Model was not yet loaded, load it and return
		loadModel(filename, shaderSet);
		
		return *m_fbxModels.find(filename)->second->getModel();
		//Logger::Error("Tried to access an fbx model that was not loaded. (" + filename + ") \n Use Application::getInstance()->getResourceManager().LoadFBXModel(" + filename + ") before accessing it.");
	}

	return *pos->second->getModel();
}
bool ResourceManager::hasModel(const std::string& filename) {
	return m_fbxModels.find(filename) != m_fbxModels.end();
}

//void ResourceManager::reloadShaders() {
//	for (auto it = m_shaderSets.begin(); it != m_shaderSets.end(); ++it)
//		it->second->reload();
//}


// Sound Manager
SoundManager* ResourceManager::getSoundManager() {
	return m_soundManager.get();
}