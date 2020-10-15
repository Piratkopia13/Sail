#pragma once

#include <Map>
#include <memory>
#include "TextureData.h"
#include "Sail/api/Texture.h"
#include "ParsedScene.h"
#include "Sail/api/shader/Shader.h"
#include "Sail/api/shader/PipelineStateObject.h"
#include "Sail/graphics/shader/Shaders.h"
#include "../api/Mesh.h"

class ResourceManager {
public:
	static const std::string MISSING_TEXTURE_NAME;
	static const std::string MISSING_TEXTURECUBE_NAME;

	static const std::string DEFAULT_MODEL_LOCATION;
	static const std::string DEFAULT_TEXTURE_LOCATION;

public:
	ResourceManager();
	~ResourceManager();

	// TextureData
	bool loadTextureData(const std::string& filename, bool useAbsolutePath = false); // Returns false if loading failed
	TextureData& getTextureData(const std::string& filename);
	bool hasTextureData(const std::string& filename);
	bool releaseTextureData(const std::string& filename);

	// Texture
	bool loadTexture(const std::string& filename, bool useAbsolutePath = false); // Returns false if loading failed
	Texture& getTexture(const std::string& filename);
	bool hasTexture(const std::string& filename);

	// Meshes
	Mesh::SPtr loadMesh(const std::string& filename, bool useAbsolutePath = false);
	Mesh::SPtr getMesh(const std::string& filename);
	bool hasMesh(const std::string& filename, Mesh::SPtr* outMesh = nullptr);

	// Shaders
	void loadShaderSet(Shaders::ShaderIdentifier shaderIdentifier);
	Shader& getShaderSet(Shaders::ShaderIdentifier shaderIdentifier);
	bool hasShaderSet(Shaders::ShaderIdentifier shaderIdentifier);
	void reloadShader(Shaders::ShaderIdentifier shaderIdentifier);
	void reloadAllShaders();

	// PipelineStateObjects (PSOs)
	// mesh may be null when shader is a compute shader
	PipelineStateObject& getPSO(Shader* shader, Mesh* mesh = nullptr);

	// Storage information
	uint32_t getTextureDataSize() const;
	uint32_t getTextureDataCount() const;
	uint32_t getShaderCount() const;
	uint32_t getPSOCount() const;

private:

	// Textures mapped to their filenames
	std::map<std::string, std::unique_ptr<TextureData>> m_textureDatas;
	std::map<std::string, std::unique_ptr<Texture>> m_textures;

	// Meshes mapped to their filenames
	std::map<std::string, Mesh::SPtr> m_meshes;

	// Shaders mapped to their identifiers
	std::map<Shaders::ShaderIdentifier, Shader*> m_shaders;
	std::map<Shaders::ShaderIdentifier, Shaders::ShaderSettings> m_shaderSettings;

	// PipelineStateObjects mapped to attribute and shader hashes
	std::map<uint32_t, std::unique_ptr<PipelineStateObject>> m_psos;
};