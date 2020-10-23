#include "pch.h"
#include "ResourceManager.h"
#include "Sail/api/shader/PipelineStateObject.h"
#include "../api/shader/Shader.h"
#include "../Application.h"
#include "loaders/ModelLoader.h"

const std::string ResourceManager::MISSING_TEXTURE_NAME = "missing.tga";
const std::string ResourceManager::MISSING_TEXTURECUBE_NAME = "missing_cube.dds";

const std::string ResourceManager::DEFAULT_MODEL_LOCATION = "res/models/";
const std::string ResourceManager::DEFAULT_TEXTURE_LOCATION = "res/textures/";

ResourceManager::ResourceManager() {
	// Forward shaders
	{
		Shaders::ShaderSettings settings;
		settings.filename = "forward/PBRMaterialShader.hlsl";
		settings.materialType = Material::PBR;
		settings.defaultPSOSettings.cullMode = GraphicsAPI::NO_CULLING; // No culling to work with vegetation
		settings.defaultPSOSettings.blendMode = GraphicsAPI::ALPHA;
		settings.identifier = Shaders::PBRMaterialShader;
		m_shaderSettings.insert({ settings.identifier, settings });
	}
	{
		Shaders::ShaderSettings settings;
		settings.filename = "forward/PhongMaterialShader.hlsl";
		settings.materialType = Material::PHONG;
		settings.defaultPSOSettings.cullMode = GraphicsAPI::NO_CULLING; // TODO: set back to BACKFACE
		settings.identifier = Shaders::PhongMaterialShader;
		m_shaderSettings.insert({ settings.identifier, settings });
	}
	{
		Shaders::ShaderSettings settings;
		settings.filename = "forward/OutlineShader.hlsl";
		settings.materialType = Material::OUTLINE;
		settings.defaultPSOSettings.cullMode = GraphicsAPI::FRONTFACE;
		settings.identifier = Shaders::OutlineShader;
		m_shaderSettings.insert({ settings.identifier, settings });
	}
	{
		Shaders::ShaderSettings settings;
		settings.filename = "forward/CubemapShader.hlsl";
		settings.materialType = Material::TEXTURES;
		settings.defaultPSOSettings.cullMode = GraphicsAPI::FRONTFACE;
		settings.defaultPSOSettings.depthMask = GraphicsAPI::WRITE_MASK;
		settings.identifier = Shaders::CubemapShader;
		m_shaderSettings.insert({ settings.identifier, settings });
	}
	// Deferred shaders
	{
		Shaders::ShaderSettings settings;
		settings.filename = "deferred/GeometryPassShader.hlsl";
		settings.materialType = Material::PBR;
		settings.defaultPSOSettings.numRenderTargets = 4;
		settings.defaultPSOSettings.rtFormats.insert({ 0, ResourceFormat::R16G16B16A16_FLOAT });
		settings.defaultPSOSettings.rtFormats.insert({ 1, ResourceFormat::R16G16B16A16_FLOAT });
		settings.defaultPSOSettings.cullMode = GraphicsAPI::BACKFACE;
		settings.identifier = Shaders::DeferredGeometryPassShader;
		m_shaderSettings.insert({ settings.identifier, settings });
	}
	{
		Shaders::ShaderSettings settings;
		settings.filename = "deferred/ShadingPassShader.hlsl";
		settings.materialType = Material::TEXTURES;
		settings.defaultPSOSettings.cullMode = GraphicsAPI::BACKFACE;
		settings.defaultPSOSettings.depthMask = GraphicsAPI::BUFFER_DISABLED;
		settings.identifier = Shaders::DeferredShadingPassShader;
		m_shaderSettings.insert({ settings.identifier, settings });
	}
	{
		Shaders::ShaderSettings settings;
		settings.filename = "deferred/ssao.hlsl";
		//settings.materialType = Material::CUSTOM;
		settings.materialType = Material::TEXTURES;
		settings.defaultPSOSettings.cullMode = GraphicsAPI::BACKFACE;
		settings.defaultPSOSettings.depthMask = GraphicsAPI::BUFFER_DISABLED;
		settings.defaultPSOSettings.rtFormats.insert({ 0, ResourceFormat::R8 });
		settings.identifier = Shaders::SSAOShader;
		m_shaderSettings.insert({ settings.identifier, settings });
	}


	// Compute shaders
	{
		Shaders::ShaderSettings settings;
		settings.filename = "compute/GenerateMipsCS.hlsl";
		settings.materialType = Material::NONE;
		settings.computeShaderSettings.threadGroupXScale = 1.0f / 8.0f;
		settings.computeShaderSettings.threadGroupYScale = 1.0f / 8.0f;
		settings.identifier = Shaders::GenerateMipsComputeShader;
		m_shaderSettings.insert({ settings.identifier, settings });
	}
	{
		Shaders::ShaderSettings settings;
		settings.filename = "postprocess/GaussianBlurHorizontal.hlsl";
		settings.materialType = Material::NONE;
		settings.computeShaderSettings.threadGroupXScale = 1.0f / 256.0f;
		settings.identifier = Shaders::GaussianBlurHorizontalComputeShader;
		m_shaderSettings.insert({ settings.identifier, settings });
	}
	{
		Shaders::ShaderSettings settings;
		settings.filename = "postprocess/GaussianBlurVertical.hlsl";
		settings.materialType = Material::NONE;
		settings.computeShaderSettings.threadGroupYScale = 1.0f / 256.0f;
		settings.identifier = Shaders::GaussianBlurVerticalComputeShader;
		m_shaderSettings.insert({ settings.identifier, settings });
	}

	// Raytracing shaders
	{
		Shaders::ShaderSettings settings;
		settings.filename = "dxr/HardShadows.hlsl";
		settings.materialType = Material::PBR;
		settings.identifier = Shaders::RTShader;
		m_shaderSettings.insert({ settings.identifier, settings });
	}
}
ResourceManager::~ResourceManager() {
	for (auto it : m_shaders) {
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
// Textures
//

void ResourceManager::loadTexture(const std::string& filename, bool useAbsolutePath) {
	SAIL_PROFILE_FUNCTION();

	std::string path = (useAbsolutePath) ? filename : DEFAULT_TEXTURE_LOCATION + filename;
	if (!hasTexture(filename))
		m_textures.insert({ filename, std::unique_ptr<Texture>(Texture::Create(path)) });
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
// Meshes
//

Mesh::SPtr ResourceManager::loadMesh(const std::string& filename, bool useAbsolutePath) {
	std::string path = (useAbsolutePath) ? filename : DEFAULT_MODEL_LOCATION + filename;

	Mesh::SPtr mesh;
	if (!hasMesh(filename, &mesh)) {
		return m_meshes.insert({ filename, ModelLoader(path).getMesh() }).first->second;
	} else {
		return mesh;
	}
}

Mesh::SPtr ResourceManager::getMesh(const std::string& filename) {
	auto pos = m_meshes.find(filename);
	if (pos == m_meshes.end())
		Logger::Error("Tried to access a resource that was not loaded. (" + filename + ") \n Use Application::getInstance()->getResourceManager().loadMesh(\"" + filename + "\") before accessing it.");

	return pos->second;
}

bool ResourceManager::hasMesh(const std::string& filename, Mesh::SPtr* outMesh) {
	auto it = m_meshes.find(filename);
	if (it != m_meshes.end()) {
		if (outMesh) *outMesh = it->second;
		return true;
	}
	return false;
}

//
// Shader
//

void ResourceManager::loadShaderSet(Shaders::ShaderIdentifier shaderIdentifier) {
	m_shaders.insert({ shaderIdentifier, Shader::Create(m_shaderSettings[shaderIdentifier]) });
}

Shader& ResourceManager::getShaderSet(Shaders::ShaderIdentifier shaderIdentifier) {
	auto pos = m_shaders.find(shaderIdentifier);
	if (pos == m_shaders.end()) {
		// ShaderSet was not yet loaded, load it and return
		loadShaderSet(shaderIdentifier);
		return *m_shaders.find(shaderIdentifier)->second;
	}

	return *pos->second;
}

bool ResourceManager::hasShaderSet(Shaders::ShaderIdentifier shaderIdentifier) {
	return m_shaders.find(shaderIdentifier) != m_shaders.end();
}

void ResourceManager::reloadShader(Shaders::ShaderIdentifier shaderIdentifier) {
	auto it = m_shaders.find(shaderIdentifier);
	if (it == m_shaders.end()) {
		Logger::Log("Cannot reload shader " + std::to_string(shaderIdentifier) + " since it is not loaded in the first place.");
		return;
	}
	Shader* shader = it->second;
	shader->~Shader(); // Delete old shader
	Shader::Create(m_shaderSettings[shaderIdentifier], shader); // Allocate new shader on the same memory address as the old
	Logger::Log("Reloaded shader " + std::to_string(shaderIdentifier));
}

void ResourceManager::reloadAllShaders() {
	Application::getInstance()->getAPI()->waitForGPU();
	for (auto& it : m_shaders) {
		Shader* shader = it.second;

		// Method 1
		// Allows shader to store critical data during reload (currently only used in Vulkan)
		{
			shader->recompile();
		}
		// Method 2
		// Recreates instance - allows changing vertex layout, buffers, register slots etc. in reloaded shaders
		{
			//shader->~Shader(); // Delete old shader
			//Shader::Create(m_shaderSettings[it.first], shader); // Allocate new shader on the same memory address as the old
		}
		Logger::Log("Reloaded shader " + std::to_string(it.first));
	}
	// Existing PSO's are now invalid since their shader has reloaded
	m_psos.clear();
}

//
// PipelineStateObjects (PSOs)
//

PipelineStateObject& ResourceManager::getPSO(Shader* shader, Mesh* mesh) {
	uint32_t hash = shader->getID() * 10e5;
	uint32_t meshHash = 0;
	if (mesh) {
		// Return a PSO that matches the attribute order in the mesh and the shader
		// The combination is hashed in a uint like "xxxxxyyyyy" where x is the shader id and y is the attribute hash
		meshHash = mesh->getAttributesHash();
		assert(shader->getID() < 21473 && "Too many Shader instances exist, how did that even happen?");
		hash += meshHash;
	} else {
		assert((shader->isComputeShader() || shader->isRayTracingShader()) && "A mesh has to be specified for all shader types except compute and ray tracing shaders when getting a PSO");
	}

	auto pos = m_psos.find(hash);
	if (pos == m_psos.end()) {
		// ShaderSet was not yet loaded, load it and return
		return *m_psos.insert({ hash, std::unique_ptr<PipelineStateObject>(PipelineStateObject::Create(shader, meshHash)) }).first->second;
	}

	return *pos->second;
}

//
// Storage information
//

uint32_t ResourceManager::getTextureDataSize() const {
	uint32_t total = 0;
	for (const auto& it : m_textureDatas) {
		total += it.second->getAllocatedMemorySize();
	}
	return total;
}

uint32_t ResourceManager::getTextureDataCount() const {
	return m_textureDatas.size();
}

uint32_t ResourceManager::getShaderCount() const {
	return m_shaders.size();
}

uint32_t ResourceManager::getPSOCount() const {
	return m_psos.size();
}
