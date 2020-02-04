#pragma once

#include "Sail/api/shader/ConstantBuffer.h"
#include "Sail/api/shader/Sampler.h"
#include "Sail/api/GraphicsAPI.h"
#include "Sail/graphics/geometry/Model.h"
#include "Sail/graphics/camera/Camera.h"
#include "Sail/utils/Utils.h"
#include "InputLayout.h"
#include "../RenderableTexture.h"

class ShaderPipeline {
public:
	friend class Shader;
	static ShaderPipeline* CurrentlyBoundShader;
	static const std::string DEFAULT_SHADER_LOCATION;

public:
	static ShaderPipeline* Create(const std::string& filename);
	ShaderPipeline(const std::string& filename);
	virtual ~ShaderPipeline();

	// The following static methods are to be implemented in APIs

	// Returns false if already bound
	virtual bool bind(void* cmdList = nullptr) = 0;
	virtual void unbind();
	// filepath is used for include paths and error messages 
	virtual void* compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) = 0;
	virtual void setTexture(const std::string& name, Texture* texture, void* cmdList = nullptr) = 0;
	virtual void setRenderableTexture(const std::string& name, RenderableTexture* texture, void* cmdList = nullptr) = 0;

	virtual void updateCamera(Camera& cam) {};
	virtual void setClippingPlane(const glm::vec4& clippingPlane) {};
	virtual void setWireframe(bool wireframeState);
	virtual void setCullMode(GraphicsAPI::Culling newCullMode);
	virtual void setNumRenderTargets(unsigned int numRenderTargets);
	virtual void enableDepthStencil(bool enable);
	virtual void enableDepthWriting(bool enable);
	virtual void setBlending(GraphicsAPI::Blending blendMode);

	bool isComputeShader() const;
	InputLayout& getInputLayout();
	void* getVsBlob();
	const std::string& getName() const;
	RenderableTexture* getRenderableTexture(const std::string& name) const;

	virtual void setCBufferVar(const std::string& name, const void* data, UINT size);
	virtual bool trySetCBufferVar(const std::string& name, const void* data, UINT size);

protected:
	// Binds shader resources using specified arguments
	bool bindInternal(void* cmdList, unsigned int meshIndex, bool forceIfBound);
	bool trySetCBufferVarInternal(const std::string& name, const void* data, UINT size, unsigned int meshIndex);
	void setCBufferVarInternal(const std::string& name, const void* data, UINT size, unsigned int meshIndex);

	// Compiles shaders into blobs
	virtual void compile();
	// Called after the inputLayout is created
	virtual void finish();

protected:
	std::unique_ptr<InputLayout> inputLayout;
	std::string filename;

	bool wireframe;
	GraphicsAPI::Culling cullMode;
	unsigned int numRenderTargets;
	bool enableDepth;
	bool enableDepthWrite;
	GraphicsAPI::Blending blendMode;

	void* vsBlob; // Used for the input layout
	void* gsBlob;
	void* psBlob;
	void* dsBlob;
	void* hsBlob;
	void* csBlob;

	struct ShaderResource {
		ShaderResource(const std::string& name, UINT slot)
			: name(name)
			, slot(slot)
		{}
		std::string name;
		UINT slot;
	};
	struct ShaderCBuffer {
		struct CBufferVariable {
			std::string name;
			UINT byteOffset;
		};
		ShaderCBuffer(std::vector<ShaderCBuffer::CBufferVariable>& vars, void* initData, UINT size, ShaderComponent::BIND_SHADER bindShader, UINT slot, bool inComputeShader)
			: vars(vars)
		{
			cBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(ShaderComponent::ConstantBuffer::Create(initData, size, bindShader, slot, inComputeShader));
		}
		std::vector<CBufferVariable> vars;
		std::unique_ptr <ShaderComponent::ConstantBuffer> cBuffer;
	};
	struct ShaderSampler {
		ShaderSampler(ShaderResource res, Texture::ADDRESS_MODE adressMode, Texture::FILTER filter, ShaderComponent::BIND_SHADER bindShader, UINT slot)
			: res(res)
		{
			sampler = std::unique_ptr<ShaderComponent::Sampler>(ShaderComponent::Sampler::Create(adressMode, filter, bindShader, slot));

		}
		ShaderResource res;
		std::unique_ptr<ShaderComponent::Sampler> sampler;
	};
	struct ShaderRenderableTexture {
		ShaderRenderableTexture(ShaderResource res, Texture::FORMAT format, const std::string& nameSuffix = "")
			: res(res)
		{
			renderableTexture = std::unique_ptr<RenderableTexture>(RenderableTexture::Create(320, 180, "Renderable Texture owned by a ShaderPipeline" + nameSuffix, format));
		}
		ShaderResource res;
		std::unique_ptr<RenderableTexture> renderableTexture;
	};
	struct ParsedData {
		bool hasVS = false, hasPS = false, hasGS = false, hasDS = false, hasHS = false, hasCS = false;
		std::vector<ShaderCBuffer> cBuffers;
		std::vector<ShaderSampler> samplers;
		std::vector<ShaderResource> textures;
		std::vector<ShaderRenderableTexture> renderableTextures;
		void clear() {
			hasVS = false; hasPS = false; hasGS = false; hasDS = false; hasHS = false, hasCS = false;
			cBuffers.clear();
			samplers.clear();
			textures.clear();
			renderableTextures.clear();
		}
	};
	ParsedData parsedData;

private:
	void parse(const std::string& source);
	void parseCBuffer(const std::string& source);
	void parseSampler(const char* source);
	void parseTexture(const char* source);
	void parseRWTexture(const char* source);
	std::string nextTokenAsName(const char* source, UINT& outTokenSize, bool allowArray = false) const;
	std::string nextTokenAsType(const char* source, UINT& outTokenSize) const;
	ShaderComponent::BIND_SHADER getBindShaderFromName(const std::string& name) const;
	
	UINT getSizeOfType(const std::string& typeName) const;

protected:
	UINT findSlotFromName(const std::string& name, const std::vector<ShaderResource>& resources) const;
};