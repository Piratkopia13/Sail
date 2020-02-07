#pragma once

#include <Map>
#include <memory>
#include "TextureData.h"
#include "Sail/api/Texture.h"
#include "ParsedScene.h"

class ShaderPipeline;
class Shader;

class ResourceManager {
public:
	ResourceManager();
	~ResourceManager();

	// TextureData
	void loadTextureData(const std::string& filename, bool useAbsolutePath = false);
	TextureData& getTextureData(const std::string& filename);
	bool hasTextureData(const std::string& filename);
	bool releaseTextureData(const std::string& filename);

	// Texture
	void loadTexture(const std::string& filename, bool useAbsolutePath = false);
	Texture& getTexture(const std::string& filename);
	bool hasTexture(const std::string& filename);

	// Models
	void loadModel(const std::string& filename, Shader* shader, bool useAbsolutePath = false);
	std::shared_ptr<Model> getModel(const std::string& filename, Shader* shader, bool useAbsolutePath = false);
	bool hasModel(const std::string& filename);

	// ShaderSets
	template <typename T>
	void loadShaderSet();
	template <typename T>
	T& getShaderSet();
	template <typename T>
	bool hasShaderSet();
	template <typename T>
	void reloadShader();

private:
	// Textures mapped to their filenames
	std::map<std::string, std::unique_ptr<TextureData>> m_textureDatas;
	std::map<std::string, std::unique_ptr<Texture>> m_textures;
	// Models mapped to their filenames
	std::map<std::string, std::unique_ptr<ParsedScene>> m_fbxModels;
	// ShaderSets mapped to their identifiers
	std::map<std::string, Shader*> m_shaderSets;
};

template <typename T>
void ResourceManager::loadShaderSet() {
	// Insert and get the new ShaderSet
	//m_shaderSets.insert({ typeid(T).name(), std::make_unique<T>() });
	m_shaderSets.insert({ typeid(T).name(), SAIL_NEW T() });
}

template <typename T>
T& ResourceManager::getShaderSet() {
	auto pos = m_shaderSets.find(typeid(T).name());
	if (pos == m_shaderSets.end()) {
		// ShaderSet was not yet loaded, load it and return
		loadShaderSet<T>();
		return dynamic_cast<T&>(*(m_shaderSets.find(typeid(T).name())->second));
	}

	return dynamic_cast<T&>(*pos->second);
}

template <typename T>
bool ResourceManager::hasShaderSet() {
	return m_shaderSets.find(typeid(T).name()) != m_shaderSets.end();
}

template <typename T>
void ResourceManager::reloadShader() {
	std::string name = typeid(T).name();
	auto it = m_shaderSets.find(name);
	if (it == m_shaderSets.end()) {
		Logger::Log("Cannot reload shader " + name + " since it is not loaded in the first place.");
		return;
	}
	T* shader = dynamic_cast<T*>(it->second);
	shader->~T();
	shader = new (shader) T();
	Logger::Log("Reloaded shader " + name);
}
