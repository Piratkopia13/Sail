#pragma once

#include <Map>
#include <memory>
#include "TextureData.h"
#include "Sail/api/Texture.h"
#include "ParsedScene.h"
#include "loaders/AssimpLoader.h"

//class DeferredGeometryShader;
class ShaderPipeline;
class Shader;
//class SoundManager;

class ResourceManager {
public:
	ResourceManager();
	~ResourceManager();
	static const std::string SAIL_DEFAULT_MODEL_LOCATION;
	

	// TextureData
	void loadTextureData(const std::string& filename);
	TextureData& getTextureData(const std::string& filename);
	bool hasTextureData(const std::string& filename);

	// Texture
	void loadTexture(const std::string& filename);
	Texture& getTexture(const std::string& filename);
	bool hasTexture(const std::string& filename);

	// Models
	void loadModel(const std::string& filename, Shader* shader);
	Model& getModel(const std::string& filename, Shader* shader);
	bool hasModel(const std::string& filename);

	// Animations
	void loadAnimationStack(const std::string& fileName);
	//AnimationStack& getAnimationStack(const std::string& fileName);
	bool hasAnimationStack(const std::string& fileName);

	// ShaderSets
	template <typename T>
	void loadShaderSet();

	template <typename T>
	T& getShaderSet() {
		auto pos = m_shaderSets.find(typeid(T).name());
		if (pos == m_shaderSets.end()) {
			// ShaderSet was not yet loaded, load it and return
			loadShaderSet<T>();
			return dynamic_cast<T&>(*(m_shaderSets.find(typeid(T).name())->second));
		}

		return dynamic_cast<T&>(*pos->second);
	}
	template <typename T>
	bool hasShaderSet() {
		return m_shaderSets.find(typeid(T).name()) != m_shaderSets.end();
	}

	template <typename T>
	void reloadShader() {
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

	// SoundManager
	//SoundManager* getSoundManager();

private:
	// Textures mapped to their filenames
	std::map<std::string, std::unique_ptr<TextureData>> m_textureDatas;
	std::map<std::string, std::unique_ptr<Texture>> m_textures;
	// Models mapped to their filenames
	std::map<std::string, std::unique_ptr<ParsedScene>> m_fbxModels;
	std::map < std::string, std::unique_ptr<Model>> m_models;
	std::map < std::string, std::unique_ptr<AnimationStack>> m_animationStacks;
	// ShaderSets mapped to their identifiers
	std::map<std::string, Shader*> m_shaderSets;
	// SoundManager containing all sounds
	//std::unique_ptr<SoundManager> m_soundManager;


	std::unique_ptr<AssimpLoader> m_assimpLoader;
};

template <typename T>
void ResourceManager::loadShaderSet() {
	// Insert and get the new ShaderSet
	//m_shaderSets.insert({ typeid(T).name(), std::make_unique<T>() });
	m_shaderSets.insert({ typeid(T).name(), SAIL_NEW T() });
}
