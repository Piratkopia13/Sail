#pragma once

#include <memory>
#include <string>
#include "Sail/api/shader/ShaderParser.h"
#include "Sail/graphics/shader/Shaders.h"

class Shader {
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

	// These really do not make any sense in modern APIs
	//virtual bool setTexture(const std::string& name, Texture* texture, void* cmdList = nullptr) = 0;
	//virtual void setRenderableTexture(const std::string& name, RenderableTexture* texture, void* cmdList = nullptr) = 0;

	virtual void setCBufferVar(const std::string& name, const void* data, unsigned int size, void* cmdList);
	virtual bool trySetCBufferVar(const std::string& name, const void* data, unsigned int size, void* cmdList);

	virtual void setClippingPlane(const glm::vec4& clippingPlane) {};

	void* getVsBlob() const;
	void* getGsBlob() const;
	void* getPsBlob() const;
	void* getDsBlob() const;
	void* getHsBlob() const;
	void* getCsBlob() const;

protected:
	// filepath is used for include paths and error messages 
	virtual void* compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) = 0;
	// Compiles shaders into blobs
	virtual void compile();

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

protected:
	void bindInternal(unsigned int meshIndex, void* cmdList) const;
	void setMaterialType(Material::Type type);

	bool trySetCBufferVarInternal(const std::string& name, const void* data, unsigned int size, unsigned int meshIndex);
	void setCBufferVarInternal(const std::string& name, const void* data, unsigned int size, unsigned int meshIndex);

protected:
	static unsigned int s_id;
	ShaderParser parser;

};