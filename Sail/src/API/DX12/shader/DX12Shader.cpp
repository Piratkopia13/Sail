#include "pch.h"
#include "DX12Shader.h"
#include <d3dcommon.h>
#include <d3dcompiler.h>
#include "../DX12API.h"
#include "Sail/Application.h"
#include "DX12ConstantBuffer.h"
#include "../resources/DescriptorHeap.h"
#include "../resources/DX12Texture.h"
#include "../resources/DX12RenderableTexture.h"
#include "DX12PipelineStateObject.h"

Shader* Shader::Create(Shaders::ShaderSettings settings, Shader* allocAddr) {
	if (!allocAddr)
		return SAIL_NEW DX12Shader(settings);
	else
		return new (allocAddr) DX12Shader(settings);
}

DX12Shader::DX12Shader(Shaders::ShaderSettings settings)
	: Shader(settings)
	, m_missingTexture(static_cast<DX12Texture&>(Application::getInstance()->getResourceManager().getTexture(ResourceManager::MISSING_TEXTURE_NAME)))
	, m_missingTextureCube(static_cast<DX12Texture&>(Application::getInstance()->getResourceManager().getTexture(ResourceManager::MISSING_TEXTURECUBE_NAME)))
{
	m_context = Application::getInstance()->getAPI<DX12API>();

#ifdef USE_DXIL_COMPILER
	m_dxilCompiler.init();
#endif

	compile();
}

DX12Shader::~DX12Shader() { }

void* DX12Shader::compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) {
#ifdef USE_DXIL_COMPILER
	DXILShaderCompiler::Desc shaderDesc;

	std::wstring targetProfile;
	std::wstring version = (m_context->getSupportedFeatures().dxr1_1) ? L"6_5" : L"6_2";

	switch (shaderType) {
	case ShaderComponent::VS:
		shaderDesc.entryPoint = L"VSMain";
		targetProfile = L"vs_" + version;
		break;
	case ShaderComponent::PS:
		shaderDesc.entryPoint = L"PSMain";
		targetProfile = L"ps_" + version;
		break;
	case ShaderComponent::GS:
		shaderDesc.entryPoint = L"GSMain";
		targetProfile = L"gs_" + version;
		break;
	case ShaderComponent::CS:
		shaderDesc.entryPoint = L"CSMain";
		targetProfile = L"cs_" + version;
		break;
	case ShaderComponent::DS:
		shaderDesc.entryPoint = L"DSMain";
		targetProfile = L"ds_" + version;
		break;
	case ShaderComponent::HS:
		shaderDesc.entryPoint = L"HSMain";
		break;
	}
	shaderDesc.targetProfile = targetProfile.c_str();

#ifdef _DEBUG
	shaderDesc.compileArguments.push_back(L"/Zi"); // Debug info
#endif
	shaderDesc.compileArguments.push_back(L"/Gis"); // Declare all variables and values as precise
	shaderDesc.source = source.c_str();
	shaderDesc.sourceSize = source.length();
	auto wfilepath = std::wstring(filepath.begin(), filepath.end());
	shaderDesc.filePath = wfilepath.c_str();

	IDxcBlob* pShaders = nullptr;
	ThrowIfFailed(m_dxilCompiler.compile(&shaderDesc, &pShaders));

#else
	// "Old" compilation

	ID3DBlob* pShaders = nullptr;
	ID3DBlob* errorBlob = nullptr;
	unsigned int flags = 0;
	flags |= D3DCOMPILE_ENABLE_UNBOUNDED_DESCRIPTOR_TABLES;
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
	flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	D3D_SHADER_MACRO defines[] = { {"GAMMA_CORRECT", "1"}, {NULL, NULL} };
	HRESULT hr;
	switch (shaderType) {
	case ShaderComponent::VS:
		hr = D3DCompile(source.c_str(), source.length(), filepath.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_1", flags, 0, &pShaders, &errorBlob);
		break;
	case ShaderComponent::PS:
		hr = D3DCompile(source.c_str(), source.length(), filepath.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_1", flags, 0, &pShaders, &errorBlob);
		break;
	case ShaderComponent::DS:
		hr = D3DCompile(source.c_str(), source.length(), filepath.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "DSMain", "ds_5_1", flags, 0, &pShaders, &errorBlob);
		break;
	case ShaderComponent::HS:
		hr = D3DCompile(source.c_str(), source.length(), filepath.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "HSMain", "hs_5_1", flags, 0, &pShaders, &errorBlob);
		break;
	case ShaderComponent::GS:
		hr = D3DCompile(source.c_str(), source.length(), filepath.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "GSMain", "gs_5_1", flags, 0, &pShaders, &errorBlob);
		break;
	case ShaderComponent::CS:
		hr = D3DCompile(source.c_str(), source.length(), filepath.c_str(), defines, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_1", flags, 0, &pShaders, &errorBlob);
		break;
	}

	if (FAILED(hr)) {
		// Print shader compilation error
		if (errorBlob) {
			char* msg = (char*)(errorBlob->GetBufferPointer());
			std::stringstream ss;
			ss << "Failed to compile shader (" << filepath << ")\n";
			for (size_t i = 0; i < errorBlob->GetBufferSize(); i++) {
				ss << msg[i];
			}
			OutputDebugStringA(ss.str().c_str());
			OutputDebugStringA("\nShader source:\n");
			OutputDebugStringA(source.c_str());
			MessageBoxA(0, ss.str().c_str(), "", 0);
			errorBlob->Release();
		}
		if (pShaders)
			pShaders->Release();
		ThrowIfFailed(hr);
	}
#endif

	return pShaders;
}

void DX12Shader::updateDescriptorsAndMaterialIndices(Renderer::RenderCommandList renderCommands, const Environment& environment, const PipelineStateObject* pso, void* cmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION("Shader updateDescriptorsAndMaterialIndices");

	DescriptorUpdateInfo updateInfo = {};
	getDescriptorUpdateInfoAndUpdateMaterialIndices(renderCommands, environment, &updateInfo);

	//auto imageIndex = m_context->getSwapIndex();
	DescriptorHeap* heap = m_context->getMainGPUDescriptorHeap();
	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);
	// Create descriptors for the 2D texture array
	if (updateInfo.bindTextureArray) {
		dxCmdList->SetGraphicsRootDescriptorTable(m_context->getRootSignEntryFromRegister("t0, space1").rootSigIndex, heap->getCurrentGPUDescriptorHandle());

		for (auto* texture : updateInfo.uniqueTextures) {
			D3D12_CPU_DESCRIPTOR_HANDLE cdh;
			auto* dxTexture = static_cast<DX12Texture*>(texture);
			if (texture->isReadyToUse()) {
				cdh = dxTexture->getSrvCDH();
				dxTexture->transitionStateTo(dxCmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			} else {
				cdh = m_missingTexture.getSrvCDH();
				m_missingTexture.transitionStateTo(dxCmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			}
			m_context->getDevice()->CopyDescriptorsSimple(1, heap->getNextCPUDescriptorHandle(), cdh, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
		for (auto* rendTexture : updateInfo.uniqueRenderableTextures) {
			// Renderable textures are always ready to use
			D3D12_CPU_DESCRIPTOR_HANDLE cdh = static_cast<DX12RenderableTexture*>(rendTexture)->getSrvCDH();
			m_context->getDevice()->CopyDescriptorsSimple(1, heap->getNextCPUDescriptorHandle(), cdh, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}


		// Fill the unused descriptor slots with the missing texture
		m_missingTexture.transitionStateTo(dxCmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		for (unsigned int i = updateInfo.uniqueRenderableTextures.size(); i < TEXTURE_ARRAY_DESCRIPTOR_COUNT; i++) {
			m_context->getDevice()->CopyDescriptorsSimple(1, heap->getNextCPUDescriptorHandle(), m_missingTexture.getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
	}

	// Create descriptors for the texture cube array
	if (updateInfo.bindTextureCubeArray) {
		dxCmdList->SetGraphicsRootDescriptorTable(m_context->getRootSignEntryFromRegister("t0, space2").rootSigIndex, heap->getCurrentGPUDescriptorHandle());

		for (auto* texture : updateInfo.uniqueTextureCubes) {
			D3D12_CPU_DESCRIPTOR_HANDLE cdh;
			auto* dxTexture = static_cast<DX12Texture*>(texture);
			if (texture->isReadyToUse()) {
				cdh = dxTexture->getSrvCDH();
				dxTexture->transitionStateTo(dxCmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			} else {
				cdh = m_missingTextureCube.getSrvCDH();
				m_missingTextureCube.transitionStateTo(dxCmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
			}
			m_context->getDevice()->CopyDescriptorsSimple(1, heap->getNextCPUDescriptorHandle(), cdh, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}


		// Fill the unused descriptor slots with the missing texture
		m_missingTextureCube.transitionStateTo(dxCmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		for (unsigned int i = updateInfo.uniqueTextureCubes.size(); i < TEXTURE_ARRAY_DESCRIPTOR_COUNT; i++) {
			m_context->getDevice()->CopyDescriptorsSimple(1, heap->getNextCPUDescriptorHandle(), m_missingTextureCube.getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		}
	}

}

bool DX12Shader::setConstantDerived(const std::string& name, const void* data, uint32_t size, ShaderComponent::BIND_SHADER bindShader, uint32_t byteOffset, void* cmdList) {
	assert(size % 4 == 0);
	assert(byteOffset % 4 == 0);
	if (isComputeShader()) {
		static_cast<ID3D12GraphicsCommandList*>(cmdList)->SetComputeRoot32BitConstants(m_context->getRootSignEntryFromRegister("b3").rootSigIndex, size / 4, data, byteOffset / 4);
	} else {
		static_cast<ID3D12GraphicsCommandList*>(cmdList)->SetGraphicsRoot32BitConstants(m_context->getRootSignEntryFromRegister("b3").rootSigIndex, size / 4, data, byteOffset / 4);
	}
	return true;
}

void DX12Shader::bind(void* cmdList) const {
	bindInternal(cmdList);
}

void DX12Shader::recompile() {
	parser.clearParsedData();
	compile();
}