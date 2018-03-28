#pragma once

#include <Map>
#include <memory>
#include "TextureData.h"
#include "DXTexture.h"
#include "parsers/FBXParser.h"

class DeferredGeometryShader;
class FbxModel;
class ShaderSet;
class SoundManager;

class ResourceManager {
public:
	ResourceManager();
	~ResourceManager();

	// TextureData
	void LoadTextureData(const std::string& filename);
	TextureData& getTextureData(const std::string& filename);
	bool hasTextureData(const std::string& filename);

	// DXTexture
	void LoadDXTexture(const std::string& filename);
	DXTexture& getDXTexture(const std::string& filename);
	bool hasDXTexture(const std::string& filename);

	// FBXParser
	FBXParser& getFBXParser();

	// FBXModels
	void LoadFBXModel(const std::string& filename);
	FbxModel& getFBXModel(const std::string& filename);
	bool hasFBXModel(const std::string& filename);

	// ShaderSets
	template <typename T>
	void LoadShaderSet() {
		// Insert and get the new ShaderSet
		//m_shaderSets.insert({ typeid(T).name(), std::make_unique<T>() });
		m_shaderSets.insert({ typeid(T).name(), new T() });
	}

	template <typename T>
	T& getShaderSet() {
		auto pos = m_shaderSets.find(typeid(T).name());
		if (pos == m_shaderSets.end()) {
			// ShaderSet was not yet loaded, load it and return
			LoadShaderSet<T>();
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
		T* shader = dynamic_cast<T*>(it->second);
		shader->~T();
		shader = new (shader) T();
		Logger::Log("Reloaded shader " + name);
	}

	// SoundManager
	SoundManager* getSoundManager();

private:
	// DONT MOVE THE NEXT LINE, WILL CAUSE CRASHES
	FBXParser m_fbxParser;

	// Textures mapped to their filenames
	std::map<std::string, std::unique_ptr<TextureData>> m_textureDatas;
	std::map<std::string, std::unique_ptr<DXTexture>> m_dxTextures;
	// Models mapped to their filenames
	std::map<std::string, std::unique_ptr<FbxModel>> m_fbxModels;
	// ShaderSets mapped to their identifiers
	std::map<std::string, ShaderSet*> m_shaderSets;
	// SoundManager containing all sounds
	std::unique_ptr<SoundManager> m_soundManager;

};