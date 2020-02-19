#include "pch.h"
#include "DX11Shader.h"
#include <d3dcommon.h>
#include "Sail/Application.h"
#include "../DX11API.h"
#include "../resources/DX11Texture.h"
#include "../resources/DX11RenderableTexture.h"

Shader* Shader::Create(Shaders::ShaderSettings settings, Shader* allocAddr) {
	if (!allocAddr)
		return SAIL_NEW DX11Shader(settings);
	else
		return new (allocAddr) DX11Shader(settings);
}

DX11Shader::DX11Shader(Shaders::ShaderSettings settings)
	: Shader(settings)
	, m_vs(nullptr)
	, m_ps(nullptr)
	, m_ds(nullptr)
	, m_hs(nullptr)
	, m_gs(nullptr)
	, m_cs(nullptr)
{
	compile();
}

DX11Shader::~DX11Shader() {
	Memory::SafeRelease(m_vs);
	Memory::SafeRelease(m_ps);
	Memory::SafeRelease(m_ds);
	Memory::SafeRelease(m_hs);
	Memory::SafeRelease(m_gs);
	Memory::SafeRelease(m_cs);
}

void DX11Shader::bind(void* cmdList) const {
	bindInternal(0U, cmdList);

	auto* devCon = Application::getInstance()->getAPI<DX11API>()->getDeviceContext();

	// Bind or unbind shaders
	// Shader types not used will be nullptr which will unbind any previously bound shaders of that type
	devCon->VSSetShader(m_vs, 0, 0);
	devCon->PSSetShader(m_ps, 0, 0);
	devCon->GSSetShader(m_gs, 0, 0);
	devCon->DSSetShader(m_ds, 0, 0);
	devCon->HSSetShader(m_hs, 0, 0);
	devCon->CSSetShader(m_cs, 0, 0);
}

void* DX11Shader::compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) {
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
	if (FAILED(D3DCompile(source.c_str(), source.size(), filepath.c_str(), NULL, D3D_COMPILE_STANDARD_FILE_INCLUDE, entryPoint.c_str(), shaderVersion.c_str(), D3DCOMPILE_DEBUG, 0, &shader, &errorMsg))) {
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

bool DX11Shader::setTexture(const std::string& name, Texture* texture, void* cmdList) {
	int slot = parser.findSlotFromName(name, parser.getParsedData().textures);
	if (slot == -1) return false; // Texture doesn't exist in shader

	ID3D11ShaderResourceView* srv[1] = { nullptr }; // Default to a null srv (unbinds the slot)
	if (texture) {
		srv[0] = ((DX11Texture*)texture)->getSRV();
	}
	auto* devCon = Application::getInstance()->getAPI<DX11API>()->getDeviceContext();
	if (isComputeShader())
		devCon->CSSetShaderResources(slot, 1, srv);
	else
		devCon->PSSetShaderResources(slot, 1, srv);
	return true;
}

void DX11Shader::setRenderableTexture(const std::string& name, RenderableTexture* texture, void* cmdList) {
	int slot = parser.findSlotFromName(name, parser.getParsedData().textures);

	ID3D11ShaderResourceView* srv[1] = { nullptr }; // Default to a null srv (unbinds the slot)
	if (texture) {
		srv[0] = *((DX11RenderableTexture*)texture)->getColorSRV();
	}
	auto* devCon = Application::getInstance()->getAPI<DX11API>()->getDeviceContext();
	if (isComputeShader())
		devCon->CSSetShaderResources(slot, 1, srv);
	else
		devCon->PSSetShaderResources(slot, 1, srv);
}

void DX11Shader::setRenderableTextureUAV(const std::string& name, RenderableTexture* texture) {
	int slot = parser.findSlotFromName(name, parser.getParsedData().textures);

	UINT counts[1] = { 1 };
	ID3D11UnorderedAccessView* uav[1] = { nullptr }; // Default to a null srv (unbinds the slot)
	if (texture) {
		uav[0] = *((DX11RenderableTexture*)texture)->getColorUAV();
	}
	auto* devCon = Application::getInstance()->getAPI<DX11API>()->getDeviceContext();
	if (isComputeShader())
		devCon->CSSetUnorderedAccessViews(slot, 1, uav, counts);
	else
		assert(false && "Non compute shader SRVs have to be set via OMSetRenderTargetsAndUAVs method, this is currently not supported. Maybe fix?");
}

void DX11Shader::compile() {
	Shader::compile();

	auto* device = Application::getInstance()->getAPI<DX11API>()->getDevice();

	// Emulate pipeline with individual shader objects
	if (void* blob = getVsBlob()) {
		ID3D10Blob* compiledShader = static_cast<ID3D10Blob*>(blob);
		ThrowIfFailed(device->CreateVertexShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &m_vs));
	}
	if (void* blob = getPsBlob()) {
		ID3D10Blob* compiledShader = static_cast<ID3D10Blob*>(blob);
		ThrowIfFailed(device->CreatePixelShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &m_ps));
		Memory::SafeRelease(compiledShader);
	}
	if (void* blob = getGsBlob()) {
		ID3D10Blob* compiledShader = static_cast<ID3D10Blob*>(blob);
		ThrowIfFailed(device->CreateGeometryShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &m_gs));
		Memory::SafeRelease(compiledShader);
	}
	if (void* blob = getDsBlob()) {
		ID3D10Blob* compiledShader = static_cast<ID3D10Blob*>(blob);
		ThrowIfFailed(device->CreateDomainShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &m_ds));
		Memory::SafeRelease(compiledShader);
	}
	if (void* blob = getHsBlob()) {
		ID3D10Blob* compiledShader = static_cast<ID3D10Blob*>(blob);
		ThrowIfFailed(device->CreateHullShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &m_hs));
		Memory::SafeRelease(compiledShader);
	}
	if (void* blob = getCsBlob()) {
		ID3D10Blob* compiledShader = static_cast<ID3D10Blob*>(blob);
		ThrowIfFailed(device->CreateComputeShader(compiledShader->GetBufferPointer(), compiledShader->GetBufferSize(), NULL, &m_cs));
		Memory::SafeRelease(compiledShader);
	}
}

