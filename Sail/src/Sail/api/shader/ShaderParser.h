#pragma once

#include "Sail/api/shader/ConstantBuffer.h"
#include "Sail/api/shader/Sampler.h"
#include "Sail/api/RenderableTexture.h"
#include "Sail/api/GraphicsAPI.h"
#include "Sail/graphics/camera/Camera.h"
#include "Sail/utils/Utils.h"
#include "InputLayout.h"

class ShaderParser {
public:
	struct ShaderResource {
		ShaderResource(const std::string& name, unsigned int slot, unsigned int vkBinding)
			: name(name)
			, slot(slot)
			, vkBinding(vkBinding)
		{ }
		std::string name;
		unsigned int slot;
		unsigned int vkBinding;
	};
	struct ShaderTexture {
		ShaderTexture(const ShaderResource& res, unsigned int arraySize = 1, bool isWritable = false, bool isTexturesArray = false, bool isTextureCubesArray = false)
			: res(res)
			, arraySize(arraySize)
			, isWritable(isWritable)
			, isTexturesArray(isTexturesArray)
			, isTextureCubesArray(isTextureCubesArray) 
		{ }
		ShaderResource res;
		unsigned int arraySize;
		bool isWritable;
		bool isTexturesArray; // True if all 2D textures used in the scene should be bound to this
		bool isTextureCubesArray; // True if all cube textures used in the scene should be bound to this
	};
	struct ShaderCBuffer {
		struct CBufferVariable {
			std::string name;
			unsigned int byteOffset;
		};
		ShaderCBuffer(const std::string& name, std::vector<ShaderCBuffer::CBufferVariable>& vars, void* initData, unsigned int size, ShaderComponent::BIND_SHADER bindShader, unsigned int slot, bool isMaterialArray, bool inComputeShader)
			: name(name)
			, vars(vars)
			, bindShader(bindShader)
			, isMaterialArray(isMaterialArray)
		{
			cBuffer = std::unique_ptr<ShaderComponent::ConstantBuffer>(ShaderComponent::ConstantBuffer::Create(initData, size, bindShader, slot, inComputeShader));
		}
		std::string name;
		std::vector<CBufferVariable> vars;
		std::unique_ptr<ShaderComponent::ConstantBuffer> cBuffer;
		ShaderComponent::BIND_SHADER bindShader;
		bool isMaterialArray;
	};
	struct ShaderSampler {
		ShaderSampler(const ShaderResource& res, Texture::ADDRESS_MODE adressMode, Texture::FILTER filter, ShaderComponent::BIND_SHADER bindShader, unsigned int slot)
			: res(res) {
			sampler = std::unique_ptr<ShaderComponent::Sampler>(ShaderComponent::Sampler::Create(adressMode, filter, bindShader, slot));

		}
		ShaderResource res;
		std::unique_ptr<ShaderComponent::Sampler> sampler;
	};
	struct ShaderRenderableTexture {
		ShaderRenderableTexture(const ShaderResource& res, ResourceFormat::TextureFormat format, const std::string& nameSuffix = "")
			: res(res) {
			renderableTexture = std::unique_ptr<RenderableTexture>(RenderableTexture::Create(320, 180, RenderableTexture::USAGE_GENERAL, "Renderable Texture owned by a ShaderPipeline" + nameSuffix, format));
		}
		ShaderResource res;
		std::unique_ptr<RenderableTexture> renderableTexture;
	};
	struct ShaderConstant {
		ShaderConstant(std::vector<ShaderCBuffer::CBufferVariable>& vars, unsigned int size, ShaderComponent::BIND_SHADER bindShader) 
			: vars(vars)
			, size(size)
			, bindShader(bindShader)
		{ }
		unsigned int size;
		std::vector<ShaderCBuffer::CBufferVariable> vars;
		ShaderComponent::BIND_SHADER bindShader;
	};
	struct ParsedData {
		bool hasVS = false, hasPS = false, hasGS = false, hasDS = false, hasHS = false, hasCS = false, hasRayGen = false, hasRayClosestHit = false, hasRayMiss = false;
		unsigned int attributesHash = 0;
		std::vector<ShaderCBuffer> cBuffers;
		std::vector<ShaderSampler> samplers;
		std::vector<ShaderTexture> textures;
		std::vector<ShaderRenderableTexture> renderableTextures;
		std::vector<ShaderConstant> constants; // Called root constants in DX12
		std::vector<ShaderResource> accelerationStructures;
		void clear() {
			attributesHash = 0;
			hasVS = false; hasPS = false; hasGS = false; hasDS = false; hasHS = false, hasCS = false, hasRayGen = false, hasRayClosestHit = false, hasRayMiss = false;
			cBuffers.clear();
			samplers.clear();
			textures.clear();
			renderableTextures.clear();
			constants.clear();
		}
	};
public:
	ShaderParser(const std::string& filename);

	std::string parse(const std::string& source);

	const ParsedData& getParsedData() const;
	void clearParsedData();
	int findSlotFromName(const std::string& name, const std::vector<ShaderResource>& resources) const;

private:
	void parseConstantBuffer(const char* src); // Parses the first way cbuffers can be defined
	void parseCBuffer(const char* src, bool storeAsConstant = false); // Parses the second way cbuffers can be defined
	void parseSampler(const char* sourceChar); // This method is allowed to change m_cleanSource!
	void parseTexture(const char* source);
	void parseRWTexture(const char* source);
	void parseAccelerationStructure(const char* source);

	bool getVkBinding(const char* lineStart, unsigned int& outVkBinding) const;
	std::string nextTokenAsName(const char* source, unsigned int& outTokenSize, int* arrayElements = nullptr) const;
	std::string nextTokenAsType(const char* source, unsigned int& outTokenSize) const;
	ShaderComponent::BIND_SHADER getBindShaderFromName(const std::string& name) const;

	unsigned int getSizeOfType(const std::string& typeName) const;

private:
	ParsedData m_parsedData;
	std::string m_filename;
	std::string m_cleanSource;
	
};