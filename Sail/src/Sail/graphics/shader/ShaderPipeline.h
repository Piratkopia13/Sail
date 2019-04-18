#pragma once

#include <memory>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <iostream>
#include <glm/glm.hpp>

#include "Sail/api/Shader.h"

#include "component/ConstantBuffer.h"
#include "component/Sampler.h"
#include "VertexShader.h"
#include "GeometryShader.h"
#include "PixelShader.h"
#include "ComputeShader.h"
#include "DomainShader.h"
#include "HullShader.h"
#include "../geometry/Model.h"
#include "../camera/Camera.h"
#include "../../api/shader/InputLayout.h"

namespace {
	static const std::string DEFAULT_SHADER_LOCATION = "res/shaders/";
}

class ShaderPipeline {

	friend class Text;

public:
	static ShaderPipeline* CurrentlyBoundShader;
public:
	ShaderPipeline(const std::string& filename);
	virtual ~ShaderPipeline();

	/*virtual void bind();
	virtual void bindCS(UINT csIndex = 0);*/

	virtual void updateCamera(Camera& cam) {};
	virtual void setClippingPlane(const glm::vec4& clippingPlane) {};

	static void* CompileShader(const std::string& source, ShaderComponent::BIND_SHADER shaderType);
	//static ID3D10Blob* compileShader(const std::string& source, const std::string& entryPoint, const std::string& shaderVersion); // Remove, its replaced by the static version above
	InputLayout& getInputLayout();

	void setCBufferVar(const std::string& name, const void* data, UINT size);
	bool trySetCBufferVar(const std::string& name, const void* data, UINT size);
	//void setTexture2D(const std::string& name, ID3D11ShaderResourceView* srv); // FIX

protected:
	void setComputeShaders(ID3D10Blob** blob, UINT numBlobs);

protected:
	std::unique_ptr<InputLayout> inputLayout;
	//ID3D10Blob* VSBlob;
	std::string filename;

private:
	/*std::unique_ptr<VertexShader> m_vs;
	std::unique_ptr<GeometryShader> m_gs;
	std::unique_ptr<PixelShader> m_ps;
	std::unique_ptr<DomainShader> m_ds;
	std::unique_ptr<HullShader> m_hs;*/
	std::vector<std::unique_ptr<ComputeShader>> m_css;
	std::unique_ptr<Shader> m_shaders;

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
		ShaderCBuffer(std::vector<ShaderCBuffer::CBufferVariable>& vars, void* initData, UINT size, ShaderComponent::BIND_SHADER bindShader, UINT slot)
			: vars(vars)
			, cBuffer(initData, size, bindShader, slot)
		{}
		std::vector<CBufferVariable> vars;
		ShaderComponent::ConstantBuffer cBuffer;
	};
	struct ShaderSampler {
		ShaderSampler(ShaderResource res, Texture::ADDRESS_MODE adressMode, Texture::FILTER filter, ShaderComponent::BIND_SHADER bindShader, UINT slot)
			: res(res)
			, sampler(adressMode, filter, bindShader, slot)
		{}
		ShaderResource res;
		ShaderComponent::Sampler sampler;
	};
	struct ParsedData {
		bool hasVS = false, hasPS = false, hasGS = false, hasDS = false, hasHS = false;
		std::vector<ShaderCBuffer> cBuffers;
		std::vector<ShaderSampler> samplers;
		std::vector<ShaderResource> textures;
		void clear() {
			hasVS = false; hasPS = false; hasGS = false; hasDS = false; hasHS = false;
			cBuffers.clear();
			samplers.clear();
			textures.clear();
		}
	};
	ParsedData m_parsedData;

private:
	void parse(const std::string& source);
	void parseCBuffer(const std::string& source);
	void parseSampler(const char* source);
	void parseTexture(const char* source);
	std::string nextTokenAsName(const char* source, UINT& outTokenSize, bool allowArray = false) const;
	ShaderComponent::BIND_SHADER getBindShaderFromName(const std::string& name) const;

	/*void setVertexShader(void* blob);
	void setGeometryShader(void* blob);
	void setPixelShader(void* blob);
	void setDomainShader(void* blob);
	void setHullShader(void* blob);*/

	UINT getSizeOfType(const std::string& typeName) const;
	UINT findSlotFromName(const std::string& name, const std::vector<ShaderResource>& resources) const;
};