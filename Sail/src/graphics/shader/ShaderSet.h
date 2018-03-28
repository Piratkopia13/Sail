#pragma once

#include <memory>
#include <Windows.h>
#include <d3d11.h>
#include <d3dcompiler.h>
#include <iostream>
#include <SimpleMath.h>

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
#include "InputLayout.h"

namespace {
	static const std::string DEFAULT_SHADER_LOCATION = "res/shaders/";
}

class ShaderSet {

	friend class Text;

public:
	static ShaderSet* CurrentlyBoundShader;
public:
	ShaderSet(const std::string& filename);
	virtual ~ShaderSet();

	virtual void bind();
	virtual void bindCS(UINT csIndex = 0);

	// TODO: remove these from all shaders
	virtual void draw(Model& model, bool bindFirst = true) {}
	virtual void draw(bool bindFirst = true) {}

	// TODO: remove this from all shaders
	virtual void createBufferFromModelData(ID3D11Buffer** vertexBuffer, ID3D11Buffer** indexBuffer, ID3D11Buffer** instanceBuffer, const void* data) = 0;

	virtual void updateCamera(Camera& cam) {};
	virtual void setClippingPlane(const DirectX::SimpleMath::Vector4& clippingPlane) {};

	static ID3D10Blob* compileShader(const std::string& source, const std::string& entryPoint, const std::string& shaderVersion);
	const InputLayout& getInputLayout() const;

	void setCBufferVar(const std::string& name, const void* data, UINT size);
	void setTexture2D(const std::string& name, ID3D11ShaderResourceView* srv);

protected:
	void setComputeShaders(ID3D10Blob** blob, UINT numBlobs);

protected:
	InputLayout inputLayout;
	ID3D10Blob* VSBlob;
	std::string filename;

private:
	std::unique_ptr<VertexShader> m_vs;
	std::unique_ptr<GeometryShader> m_gs;
	std::unique_ptr<PixelShader> m_ps;
	std::unique_ptr<DomainShader> m_ds;
	std::unique_ptr<HullShader> m_hs;
	std::vector<std::unique_ptr<ComputeShader>> m_css;

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
		ShaderSampler(ShaderResource res, D3D11_TEXTURE_ADDRESS_MODE adressMode, D3D11_FILTER filter, ShaderComponent::BIND_SHADER bindShader, UINT slot)
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

	void setVertexShader(ID3D10Blob* blob);
	void setGeometryShader(ID3D10Blob* blob);
	void setPixelShader(ID3D10Blob* blob);
	void setDomainShader(ID3D10Blob* blob);
	void setHullShader(ID3D10Blob* blob);

	UINT getSizeOfType(const std::string& typeName) const;
	UINT findSlotFromName(const std::string& name, const std::vector<ShaderResource>& resources) const;
};