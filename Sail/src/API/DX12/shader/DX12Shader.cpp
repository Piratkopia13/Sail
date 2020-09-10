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

Shader* Shader::Create(Shaders::ShaderSettings settings, Shader* allocAddr) {
	if (!allocAddr)
		return SAIL_NEW DX12Shader(settings);
	else
		return new (allocAddr) DX12Shader(settings);
}

DX12Shader::DX12Shader(Shaders::ShaderSettings settings)
	: Shader(settings)
{
	EventSystem::getInstance()->subscribeToEvent(Event::NEW_FRAME, this);
	m_context = Application::getInstance()->getAPI<DX12API>();

	m_meshIndex[0].store(0);
	m_meshIndex[1].store(0);

#ifdef USE_DXIL_COMPILER
	m_dxilCompiler.init();
#endif

	compile();
}

DX12Shader::~DX12Shader() {
	EventSystem::getInstance()->unsubscribeFromEvent(Event::NEW_FRAME, this);
}

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
#if defined( DEBUG ) || defined( _DEBUG )
	flags |= D3DCOMPILE_DEBUG;
	flags |= D3DCOMPILE_SKIP_OPTIMIZATION;
#endif
	HRESULT hr;
	switch (shaderType) {
	case ShaderComponent::VS:
		hr = D3DCompile(source.c_str(), source.length(), filepath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_0", flags, 0, &pShaders, &errorBlob);
		break;
	case ShaderComponent::PS:
		hr = D3DCompile(source.c_str(), source.length(), filepath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_0", flags, 0, &pShaders, &errorBlob);
		break;
	case ShaderComponent::DS:
		hr = D3DCompile(source.c_str(), source.length(), filepath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "DSMain", "ds_5_0", flags, 0, &pShaders, &errorBlob);
		break;
	case ShaderComponent::HS:
		hr = D3DCompile(source.c_str(), source.length(), filepath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "HSMain", "hs_5_0", flags, 0, &pShaders, &errorBlob);
		break;
	case ShaderComponent::GS:
		hr = D3DCompile(source.c_str(), source.length(), filepath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "GSMain", "gs_5_0", flags, 0, &pShaders, &errorBlob);
		break;
	case ShaderComponent::CS:
		hr = D3DCompile(source.c_str(), source.length(), filepath.c_str(), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "CSMain", "cs_5_0", flags, 0, &pShaders, &errorBlob);
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
			OutputDebugStringA("\n");
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

void DX12Shader::instanceFinished() {
	const auto swapIndex = m_context->getSwapIndex();
	m_meshIndex[swapIndex].fetch_add(1, std::memory_order_relaxed);
}

void DX12Shader::reserve(unsigned int meshIndexMax) {
	for (auto& it : parser.getParsedData().cBuffers) {
		static_cast<ShaderComponent::DX12ConstantBuffer*>(it.cBuffer.get())->reserve(meshIndexMax);
	}
}

bool DX12Shader::onEvent(Event& event) {
	auto newFrame = [&](NewFrameEvent& event) {
		const auto swapIndex = m_context->getSwapIndex();
		m_meshIndex[swapIndex].store(0);
		return true;
	};
	EventHandler::HandleType<NewFrameEvent>(event, newFrame);
	return true;
}

void DX12Shader::bind(void* cmdList, uint32_t frameIndex) const {
	bindInternal(getMeshIndex(), cmdList);
}

unsigned int DX12Shader::getMeshIndex() const {
	const auto swapIndex = m_context->getSwapIndex();
	return m_meshIndex[swapIndex];
}

bool DX12Shader::setTexture(const std::string& name, Texture* texture, void* cmdList) {
	if (!texture) {
		// No texture bound to this slot, step past it in the heap
		m_context->getMainGPUDescriptorHeap()->getAndStepIndex(1);
		return false;
	}
	auto* dxTexture = static_cast<DX12Texture*>(texture);
	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);

	dxTexture->transitionStateTo(dxCmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	// Copy texture SRVs to the gpu heap
	// The SRV will point to a null descriptor before the texture is fully initialized, and therefor show up as black
	m_context->getDevice()->CopyDescriptorsSimple(1, m_context->getMainGPUDescriptorHeap()->getNextCPUDescriptorHandle(), dxTexture->getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	return true;
}

void DX12Shader::setRenderableTexture(const std::string& name, RenderableTexture* texture, void* cmdList) {
	if (!texture) {
		// No texture bound to this slot, step past it in the heap
		m_context->getMainGPUDescriptorHeap()->getAndStepIndex(1);
		return;
	}
	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);
	DX12RenderableTexture* dxTexture = static_cast<DX12RenderableTexture*>(texture);

	// Copy texture resource view to the gpu heap
	dxTexture->transitionStateTo(dxCmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	// Use main/graphics heap
	m_context->getDevice()->CopyDescriptorsSimple(1, m_context->getMainGPUDescriptorHeap()->getNextCPUDescriptorHandle(), dxTexture->getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void DX12Shader::setCBufferVar(const std::string& name, const void* data, unsigned int size, void* cmdList) {
	setCBufferVarInternal(name, data, size, getMeshIndex());
}

bool DX12Shader::trySetCBufferVar(const std::string& name, const void* data, unsigned int size, void* cmdList) {
	return trySetCBufferVarInternal(name, data, size, getMeshIndex());
}