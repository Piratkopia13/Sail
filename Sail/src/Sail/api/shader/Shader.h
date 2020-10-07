#pragma once

#include <memory>
#include <string>
#include "Sail/events/Events.h"
#include "Sail/api/shader/ShaderParser.h"
#include "Sail/graphics/shader/Shaders.h"

class Shader : public IEventListener {
public:
	const unsigned int TEXTURE_ARRAY_DESCRIPTOR_COUNT = 128;
	const unsigned int MATERIAL_ARRAY_DESCRIPTOR_COUNT = 1024;

public:
	// allocAddr is used to allow reloading/recreating shaders without changing their memory address
	static Shader* Create(Shaders::ShaderSettings settings, Shader* allocAddr = nullptr);

	friend class PipelineStateObject;
	static const std::string DEFAULT_SHADER_LOCATION;
	
	Shader(Shaders::ShaderSettings settings);
	virtual ~Shader();

	Material::Type getMaterialType() const;
	bool isComputeShader() const;
	unsigned int getID() const;
	unsigned int getAttributesHash() const;
	const Shaders::ShaderSettings& getSettings() const;

	virtual void bind(void* cmdList) const;
	virtual void recompile() = 0;
	
	RenderableTexture* getRenderableTexture(const std::string& name) const;

	// Builds (and sometimes binds) descriptors depending on the renderCommands about to be submitted
	virtual void updateDescriptorsAndMaterialIndices(Renderer::RenderCommandList renderCommands, const Environment& environment, const PipelineStateObject* pso, void* cmdList) = 0;

	virtual void setCBufferVar(const std::string& name, const void* data, unsigned int size, void* cmdList);
	virtual bool trySetCBufferVar(const std::string& name, const void* data, unsigned int size, void* cmdList);

	virtual void setConstantVar(const std::string& name, const void* data, unsigned int size, void* cmdList);
	virtual bool trySetConstantVar(const std::string& name, const void* data, unsigned int size, void* cmdList);

	virtual void setClippingPlane(const glm::vec4& clippingPlane) {};

	void* getVsBlob() const;
	void* getGsBlob() const;
	void* getPsBlob() const;
	void* getDsBlob() const;
	void* getHsBlob() const;
	void* getCsBlob() const;

	const ShaderParser::ParsedData& getParsedData() const;

	virtual bool onEvent(Event& event) override;

protected:
	static unsigned int s_id;
	ShaderParser parser;

	struct DescriptorUpdateInfo {
		bool bindTextureArray;
		bool bindTextureCubeArray;
		bool textureArrayIsWritable;
		bool textureCubeArrayIsWritable;
		unsigned int textureArrayBinding;
		unsigned int textureCubeArrayBinding;
		std::vector<Texture*> uniqueTextures;
		std::vector<Texture*> uniqueTextureCubes;
		std::vector<RenderableTexture*> uniqueRenderableTextures;
		ShaderComponent::ConstantBuffer* materialBuffer;
	};

protected:

	// filepath is used for include paths and error messages 
	virtual void* compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) = 0;
	// Compiles shaders into blobs
	virtual void compile();
	// Called by (try)setConstantVar
	virtual bool setConstantDerived(const std::string& name, const void* data, uint32_t size, ShaderComponent::BIND_SHADER bindShader, uint32_t byteOffset, void* cmdList = nullptr) = 0;
	
	void bindInternal(void* cmdList) const;
	void setMaterialType(Material::Type type);

	bool trySetCBufferVarInternal(const std::string& name, const void* data, unsigned int size);
	void setCBufferVarInternal(const std::string& name, const void* data, unsigned int size);

	void getDescriptorUpdateInfoAndUpdateMaterialIndices(Renderer::RenderCommandList renderCommands, const Environment& environment, DescriptorUpdateInfo* outUpdateInfo);

private:
	bool findConstant(const std::string& name, uint32_t* outOffset, ShaderComponent::BIND_SHADER* outBindShader) const;

private:
	unsigned int m_id;
	Shaders::ShaderSettings m_settings;
	std::string m_filename;
	Material::Type m_materialType;
	void* m_vsBlob;
	void* m_gsBlob;
	void* m_psBlob;
	void* m_dsBlob;
	void* m_hsBlob;
	void* m_csBlob;

	// Resources reset every frame
	// Since the same shader can be used for multiple PSOs, they need to be stored between every call to prepareToRender() this frame
	std::vector<Material*> m_uniqueMaterials;
	unsigned int m_lastMaterialIndex;
};