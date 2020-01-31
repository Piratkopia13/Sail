#include "pch.h"
#include "DX11ShaderPipeline.h"
#include "Sail/Application.h"
#include "../DX11API.h"
#include "API/DX11/resources/DX11Texture.h"

ShaderPipeline* ShaderPipeline::Create(const std::string& filename) {
	return SAIL_NEW DX11ShaderPipeline(filename);
}

DX11ShaderPipeline::DX11ShaderPipeline(const std::string& filename) 
	: ShaderPipeline(filename)
	, m_vs(nullptr)
	, m_ps(nullptr)
	, m_ds(nullptr)
	, m_hs(nullptr)
	, m_gs(nullptr)
{
	
}

DX11ShaderPipeline::~DX11ShaderPipeline() {
	ID3D10Blob* compiledShader = static_cast<ID3D10Blob*>(vsBlob);
	Memory::SafeRelease(compiledShader);

	Memory::SafeRelease(m_vs);
	Memory::SafeRelease(m_ps);
	Memory::SafeRelease(m_ds);
	Memory::SafeRelease(m_hs);
	Memory::SafeRelease(m_gs);
}

bool DX11ShaderPipeline::bind(void* cmdList, bool forceIfBound) {
	if (!ShaderPipeline::bind(cmdList, forceIfBound)) {
		return false;
	}

	auto* devCon = Application::getInstance()->getAPI<DX11API>()->getDeviceContext();

	// Bind or unbind shaders
	// Shader types not used will be nullptr which will unbind any previously bound shaders of that type
	devCon->VSSetShader(m_vs, 0, 0);
	devCon->PSSetShader(m_ps, 0, 0);
	devCon->GSSetShader(m_gs, 0, 0);
	devCon->DSSetShader(m_ds, 0, 0);
	devCon->HSSetShader(m_hs, 0, 0);

	return true;
}

void* DX11ShaderPipeline::compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) {
	std::string entryPoint = "VSMain";
	std::string shaderVersion = "vs_5_0";
	switch (shaderType) {
	case ShaderComponent::VS:
		entryPoint = "VSMain";
		shaderVersion = "vs_5_0";
		break;
	case ShaderComponent::PS:
		entryPoint = "PSMain";
		shaderVersion = "ps_5_0";
		break;
	case ShaderComponent::GS:
		entryPoint = "GSMain";
		shaderVersion = "gs_5_0";
		break;
	case ShaderComponent::CS:
		entryPoint = "CSMain";
		shaderVersion = "cs_5_0";
		break;
	case ShaderComponent::DS:
		entryPoint = "DSMain";
		shaderVersion = "ds_5_0";
		break;
	case ShaderComponent::HS:
		entryPoint = "HSMain";
		shaderVersion = "hs_5_0";
		break;
	}

	ID3D10Blob* shader = nullptr;
	ID3D10Blob* errorMsg;
	if (FAILED(D3DCompile(source.c_str(), source.size(), DEFAULT_SHADER_LOCATION.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), shaderVersion.c_str(), D3DCOMPILE_DEBUG, 0, &shader, &errorMsg))) {
		OutputDebugString(L"\n Failed to compile shader\n\n");

		char* msg = (char*)(errorMsg->GetBufferPointer());

		std::stringstream ss;
		ss << "Failed to compile shader (" << entryPoint << ", " << shaderVersion << ")\n";

		for (size_t i = 0; i < errorMsg->GetBufferSize(); i++) {
			ss << msg[i];
		}
		Logger::Error(ss.str());
	}

	return shader;
}

void DX11ShaderPipeline::setTexture2D(const std::string& name, Texture* texture, void* cmdList) {
	UINT slot = findSlotFromName(name, parsedData.textures);
	auto* srv = ((DX11Texture*)texture)->getSRV();
	Application::getInstance()->getAPI<DX11API>()->getDeviceContext()->PSSetShaderResources(slot, 1, &srv);
}

void DX11ShaderPipeline::compile() {
	ShaderPipeline::compile();

	auto* devCon = Application::getInstance()->getAPI<DX11API>()->getDeviceContext();

	// Emulate pipeline with individual shader objects
	if (vsBlob) {
		//std::cout << "has vs" << std::endl;
		ID3D10Blob* compiledShader = static_cast<ID3D10Blob*>(vsBlob);
		ThrowIfFailed(Application::getInstance()->getAPI<DX11API>()->getDevice()->CreateVertexShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &m_vs));
		//Memory::safeRelease(compiledShader);
	}
	if (psBlob) {
		//std::cout << "has ps" << std::endl;
		ID3D10Blob* compiledShader = static_cast<ID3D10Blob*>(psBlob);
		ThrowIfFailed(Application::getInstance()->getAPI<DX11API>()->getDevice()->CreatePixelShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &m_ps));
		Memory::SafeRelease(compiledShader);
	}
	if (gsBlob) {
		//std::cout << "has gs" << std::endl;
		ID3D10Blob* compiledShader = static_cast<ID3D10Blob*>(gsBlob);
		ThrowIfFailed(Application::getInstance()->getAPI<DX11API>()->getDevice()->CreateGeometryShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &m_gs));
		Memory::SafeRelease(compiledShader);
	}
	if (dsBlob) {
		//std::cout << "has ds" << std::endl;
		ID3D10Blob* compiledShader = static_cast<ID3D10Blob*>(dsBlob);
		ThrowIfFailed(Application::getInstance()->getAPI<DX11API>()->getDevice()->CreateDomainShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &m_ds));
		Memory::SafeRelease(compiledShader);
	}
	if (hsBlob) {
		//std::cout << "has hs" << std::endl;
		ID3D10Blob* compiledShader = static_cast<ID3D10Blob*>(hsBlob);
		ThrowIfFailed(Application::getInstance()->getAPI<DX11API>()->getDevice()->CreateHullShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &m_hs));
		Memory::SafeRelease(compiledShader);
	}
}
