#pragma once

#include <Map>
#include <memory>
#include "TextureData.h"
#include "Sail/api/Texture.h"
#include "ParsedScene.h"
#include "Sail/api/shader/Shader.h"
#include "Sail/api/shader/PipelineStateObject.h"
#include "Sail/graphics/shader/Shaders.h"

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

	// Shaders
	void loadShaderSet(ShaderIdentifier shaderIdentifier);
	Shader& getShaderSet(ShaderIdentifier shaderIdentifier);
	bool hasShaderSet(ShaderIdentifier shaderIdentifier);
	void reloadShader(ShaderIdentifier shaderIdentifier);

	// PipelineStateObjects (PSOs)
	// mesh may be null when shader is a compute shader
	PipelineStateObject& getPSO(Shader* shader, Mesh* mesh = nullptr);

private:
	// Textures mapped to their filenames
	std::map<std::string, std::unique_ptr<TextureData>> m_textureDatas;
	std::map<std::string, std::unique_ptr<Texture>> m_textures;
	// Models mapped to their filenames
	std::map<std::string, std::unique_ptr<ParsedScene>> m_fbxModels;
	// Shaders mapped to their identifiers
	std::map<ShaderIdentifier, Shader*> m_shaders;
	std::map<ShaderIdentifier, ShaderSettings> m_shaderSettings;
	// PipelineStateObjects mapped to attribute and shader hashes
	std::map<unsigned int, std::unique_ptr<PipelineStateObject>> m_psos;
};