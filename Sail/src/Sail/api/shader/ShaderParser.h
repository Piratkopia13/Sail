#pragma once

#include "Sail/api/shader/ConstantBuffer.h"
#include "Sail/api/shader/Sampler.h"
#include "Sail/api/RenderableTexture.h"
#include "Sail/api/GraphicsAPI.h"
#include "Sail/graphics/geometry/Model.h"
#include "Sail/graphics/camera/Camera.h"
#include "Sail/utils/Utils.h"
#include "InputLayout.h"

class ShaderParser {
private:
	struct ShaderResource {
		ShaderResource(const std::string& name, unsigned int slot, unsigned int arraySize, unsigned int vkBinding, bool isTexturesArray = false)
			: name(name)
			, slot(slot)
			, arraySize(arraySize)
			, vkBinding(vkBinding)
			, isTexturesArray(isTexturesArray)
		{ }
		std::string name;
		unsigned int slot;
		unsigned int vkBinding;
		unsigned int arraySize;
		bool isTexturesArray; // True if all textures used in the scene should be bound to this
	};
	struct ShaderCBuffer {
		struct CBufferVariable {
			std::string name;
			unsigned int byteOffset;
		};
		ShaderCBuffer(std::vector<ShaderCBuffer::CBufferVariable>& vars, void* initData, unsigned int size, ShaderComponent::BIND_SHADER bindShader, unsigned int slot, bool isMaterialArray, bool inComputeShader)
			: vars(vars)
			, bindShader(bindShader)
			, isMaterialArray(isMaterialArray)
		{
			cBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(ShaderComponent::ConstantBuffer::Create(initData, size, bindShader, slot, inComputeShader));
		}
		std::vector<CBufferVariable> vars;
		std::unique_ptr<ShaderComponent::ConstantBuffer> cBuffer;
		ShaderComponent::BIND_SHADER bindShader;
		bool isMaterialArray;
	};
	struct ShaderSampler {
		ShaderSampler(ShaderResource res, Texture::ADDRESS_MODE adressMode, Texture::FILTER filter, ShaderComponent::BIND_SHADER bindShader, unsigned int slot)
			: res(res) {
			sampler = std::unique_ptr<ShaderComponent::Sampler>(ShaderComponent::Sampler::Create(adressMode, filter, bindShader, slot));

		}
		ShaderResource res;
		std::unique_ptr<ShaderComponent::Sampler> sampler;
	};
	struct ShaderRenderableTexture {
		ShaderRenderableTexture(ShaderResource res, ResourceFormat::TextureFormat format, const std::string& nameSuffix = "")
			: res(res) {
			renderableTexture = std::unique_ptr<RenderableTexture>(RenderableTexture::Create(320, 180, "Renderable Texture owned by a ShaderPipeline" + nameSuffix, format));
		}
		ShaderResource res;
		std::unique_ptr<RenderableTexture> renderableTexture;
	};
	struct ShaderPushConstant {
		ShaderPushConstant(std::vector<ShaderCBuffer::CBufferVariable>& vars, unsigned int size, ShaderComponent::BIND_SHADER bindShader) 
			: vars(vars)
			, size(size)
			, bindShader(bindShader)
		{ }
		unsigned int size;
		std::vector<ShaderCBuffer::CBufferVariable> vars;
		ShaderComponent::BIND_SHADER bindShader;
	};
	struct ParsedData {
		bool hasVS = false, hasPS = false, hasGS = false, hasDS = false, hasHS = false, hasCS = false;
		unsigned int attributesHash = 0;
		std::vector<ShaderCBuffer> cBuffers;
		std::vector<ShaderSampler> samplers;
		std::vector<ShaderResource> textures;
		std::vector<ShaderRenderableTexture> renderableTextures;
		std::vector<ShaderPushConstant> pushConstants; // Called root constants in DX12
		void clear() {
			attributesHash = 0;
			hasVS = false; hasPS = false; hasGS = false; hasDS = false; hasHS = false, hasCS = false;
			cBuffers.clear();
			samplers.clear();
			textures.clear();
			renderableTextures.clear();
		}
	};
public:
	ShaderParser(const std::string& filename);

	std::string parse(const std::string& source);

	const ParsedData& getParsedData() const;
	int findSlotFromName(const std::string& name, const std::vector<ShaderResource>& resources) const;

private:
	void parseConstantBuffer(const std::string& source); // Parses the first way cbuffers can be defined
	void parseCBuffer(const std::string& source, bool storeAsPushConstant = false); // Parses the second way cbuffers can be defined
	void parseSampler(const char* sourceChar, std::string& source); // source argument is not const since this method is allowed to change the it!
	void parseTexture(const char* source);
	void parseRWTexture(const char* source);
	std::string nextTokenAsName(const char* source, unsigned int& outTokenSize, int* arrayElements = nullptr) const;
	std::string nextTokenAsType(const char* source, unsigned int& outTokenSize) const;
	ShaderComponent::BIND_SHADER getBindShaderFromName(const std::string& name) const;

	unsigned int getSizeOfType(const std::string& typeName) const;

private:
	ParsedData m_parsedData;
	std::string m_filename;
	std::string m_cleanSource;
	
};