#include "pch.h"
#include "DX12ShaderPipeline.h"
#include "Sail/Application.h"
#include "DX12InputLayout.h"
#include "../DX12API.h"
#include "DX12ConstantBuffer.h"
#include "../resources/DX12Texture.h"
#include "../resources/DX12RenderableTexture.h"

std::unique_ptr<DXILShaderCompiler> DX12ShaderPipeline::m_dxilCompiler = nullptr;

ShaderPipeline* ShaderPipeline::Create(const std::string& filename) {
	return SAIL_NEW DX12ShaderPipeline(filename);
}

DX12ShaderPipeline::DX12ShaderPipeline(const std::string& filename) 
	: ShaderPipeline(filename)
{
	EventSystem::getInstance()->subscribeToEvent(Event::NEW_FRAME, this);
	m_context = Application::getInstance()->getAPI<DX12API>();

	m_meshIndex[0].store(0);
	m_meshIndex[1].store(0);

	if (!m_dxilCompiler) {
		m_dxilCompiler = std::make_unique<DXILShaderCompiler>();
		m_dxilCompiler->init();
	}
}

DX12ShaderPipeline::~DX12ShaderPipeline() {
	EventSystem::getInstance()->unsubscribeFromEvent(Event::NEW_FRAME, this);
	m_context->waitForGPU();
}

bool DX12ShaderPipeline::bind(void* cmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	if (!m_pipelineState)
		Logger::Error("Tried to bind DX12PipelineState before the DirectX PipelineStateObject has been created!");
	
	// TODO: This returns false if pipeline is already bound, maybe do something with that
	bindInternal(cmdList, getMeshIndex(), true);
	
	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);
	dxCmdList->SetPipelineState(m_pipelineState.Get());

	return true;
}

void* DX12ShaderPipeline::compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) {
//#define USE_DXIL_COMPILER
#ifdef USE_DXIL_COMPILER
	DXILShaderCompiler::Desc shaderDesc;

	switch (shaderType) {
	case ShaderComponent::VS:
		shaderDesc.entryPoint = L"VSMain";
		shaderDesc.targetProfile = L"vs_6_0";
		break;
	case ShaderComponent::PS:
		shaderDesc.entryPoint = L"PSMain";
		shaderDesc.targetProfile = L"ps_6_0";
		break;
	case ShaderComponent::GS:
		shaderDesc.entryPoint = L"GSMain";
		shaderDesc.targetProfile = L"gs_6_0";
		break;
	case ShaderComponent::CS:
		shaderDesc.entryPoint = L"CSMain";
		shaderDesc.targetProfile = L"cs_6_0";
		break;
	case ShaderComponent::DS:
		shaderDesc.entryPoint = L"DSMain";
		shaderDesc.targetProfile = L"ds_6_0";
		break;
	case ShaderComponent::HS:
		shaderDesc.entryPoint = L"HSMain";
		shaderDesc.targetProfile = L"hs_6_0";
		break;
	}

#ifdef _DEBUG
	shaderDesc.compileArguments.push_back(L"/Zi"); // Debug info
#endif
	shaderDesc.compileArguments.push_back(L"/Gis"); // Declare all variables and values as precise
	shaderDesc.source = source.c_str();
	shaderDesc.sourceSize = source.length();
	auto wfilepath = std::wstring(filepath.begin(), filepath.end());
	shaderDesc.filePath = wfilepath.c_str();

	IDxcBlob* pShaders = nullptr;
	ThrowIfFailed(m_dxilCompiler->compile(&shaderDesc, &pShaders));

#else
	// "Old" compilation

	ID3DBlob* pShaders = nullptr;
	ID3DBlob* errorBlob = nullptr;
	UINT flags = 0;
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

void DX12ShaderPipeline::setTexture(const std::string& name, Texture* texture, void* cmdList) {
	auto* dxTexture = static_cast<DX12Texture*>(texture);
	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);

	dxTexture->transitionStateTo(dxCmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	// Copy texture SRVs to the gpu heap
	// The SRV will point to a null descriptor before the texture is fully initialized, and therefor show up as black
	m_context->getDevice()->CopyDescriptorsSimple(1, m_context->getMainGPUDescriptorHeap()->getNextCPUDescriptorHandle(), dxTexture->getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void DX12ShaderPipeline::setRenderableTexture(const std::string& name, RenderableTexture* texture, void* cmdList) {
	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);
	DX12RenderableTexture* dxTexture = static_cast<DX12RenderableTexture*>(texture);

	// Copy texture resource view to the gpu heap
	dxTexture->transitionStateTo(dxCmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	// Use main/graphics heap
	m_context->getDevice()->CopyDescriptorsSimple(1, m_context->getMainGPUDescriptorHeap()->getNextCPUDescriptorHandle(), dxTexture->getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void DX12ShaderPipeline::setRenderTargetFormat(unsigned int rtIndex, DXGI_FORMAT format) {
	m_rtFormats[rtIndex] = format;
}

void DX12ShaderPipeline::instanceFinished() {
	const auto swapIndex = m_context->getSwapIndex();
	m_meshIndex[swapIndex].fetch_add(1, std::memory_order_relaxed);
}

void DX12ShaderPipeline::reserve(unsigned int meshIndexMax) {
	for (auto& it : parsedData.cBuffers) {
		static_cast<ShaderComponent::DX12ConstantBuffer*>(it.cBuffer.get())->reserve(meshIndexMax);
	}
}

bool DX12ShaderPipeline::onEvent(Event& event) {
	auto newFrame = [&](NewFrameEvent& event) {
		const auto swapIndex = m_context->getSwapIndex();
		m_meshIndex[swapIndex].store(0);
		return true;
	};
	EventHandler::HandleType<NewFrameEvent>(event, newFrame);
	return true;
}

void DX12ShaderPipeline::setCBufferVar(const std::string& name, const void* data, UINT size) {
	setCBufferVarInternal(name, data, size, getMeshIndex());
}

bool DX12ShaderPipeline::trySetCBufferVar(const std::string& name, const void* data, UINT size) {
	return trySetCBufferVarInternal(name, data, size, getMeshIndex());
}

void DX12ShaderPipeline::compile() {
	ShaderPipeline::compile();
}

void DX12ShaderPipeline::finish() {
	// Create a compute pipeline state if it has a compute shader
	if (isComputeShader()) {
		createComputePipelineState();
	} else {
		createGraphicsPipelineState();
	}
}

unsigned int DX12ShaderPipeline::getMeshIndex() {
	const auto swapIndex = m_context->getSwapIndex();
	return m_meshIndex[swapIndex];
}

void DX12ShaderPipeline::createGraphicsPipelineState() {
	auto vsD3DBlob = static_cast<ID3DBlob*>(vsBlob);
	auto psD3DBlob = static_cast<ID3DBlob*>(psBlob);
	auto gsD3DBlob = static_cast<ID3DBlob*>(gsBlob);
	auto dsD3DBlob = static_cast<ID3DBlob*>(dsBlob);
	auto hsD3DBlob = static_cast<ID3DBlob*>(hsBlob);

	////// Pipeline State //////
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsd = {};

	// Specify pipeline stages
	gpsd.pRootSignature = m_context->getGlobalRootSignature();
	gpsd.InputLayout = static_cast<DX12InputLayout*>(inputLayout.get())->getDesc();
	gpsd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	if (vsBlob) {
		gpsd.VS.pShaderBytecode = reinterpret_cast<void*>(vsD3DBlob->GetBufferPointer());
		gpsd.VS.BytecodeLength = vsD3DBlob->GetBufferSize();
	}
	if (psBlob) {
		gpsd.PS.pShaderBytecode = reinterpret_cast<void*>(psD3DBlob->GetBufferPointer());
		gpsd.PS.BytecodeLength = psD3DBlob->GetBufferSize();
	}
	if (gsBlob) {
		gpsd.GS.pShaderBytecode = reinterpret_cast<void*>(gsD3DBlob->GetBufferPointer());
		gpsd.GS.BytecodeLength = gsD3DBlob->GetBufferSize();
	}
	if (dsBlob) {
		gpsd.DS.pShaderBytecode = reinterpret_cast<void*>(dsD3DBlob->GetBufferPointer());
		gpsd.DS.BytecodeLength = dsD3DBlob->GetBufferSize();
	}
	if (hsBlob) {
		gpsd.HS.pShaderBytecode = reinterpret_cast<void*>(hsD3DBlob->GetBufferPointer());
		gpsd.HS.BytecodeLength = hsD3DBlob->GetBufferSize();
	}

	// Specify render target and depthstencil usage
	for (unsigned int i = 0; i < numRenderTargets; i++) {
		if (m_rtFormats.find(i) != m_rtFormats.end()) {
			gpsd.RTVFormats[i] = m_rtFormats[i];
		} else {
			gpsd.RTVFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM;
		}
	}
	gpsd.NumRenderTargets = numRenderTargets;

	gpsd.SampleDesc.Count = 1;
	gpsd.SampleDesc.Quality = 0;
	gpsd.SampleMask = UINT_MAX;

	// Specify rasterizer behaviour
	if (wireframe) {
		gpsd.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	} else {
		gpsd.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	}

	if (cullMode == GraphicsAPI::Culling::NO_CULLING) {
		gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	} else if (cullMode == GraphicsAPI::Culling::FRONTFACE) {
		gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	} else if (cullMode == GraphicsAPI::Culling::BACKFACE) {
		gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
	}

	// Specify blend descriptions
	D3D12_RENDER_TARGET_BLEND_DESC defaultRTdesc = {
		false, false,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL
	};

	D3D12_RENDER_TARGET_BLEND_DESC customRTBlendDesc = defaultRTdesc;
	if (blendMode == GraphicsAPI::ALPHA) {
		customRTBlendDesc = {
			true, false,
			D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_ONE, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ZERO, D3D12_BLEND_ONE, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL
		};
		customRTBlendDesc.BlendEnable = TRUE;
		customRTBlendDesc.SrcBlend = D3D12_BLEND_ONE;
		customRTBlendDesc.DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
		customRTBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
		customRTBlendDesc.SrcBlendAlpha = D3D12_BLEND_ONE;
		customRTBlendDesc.DestBlendAlpha = D3D12_BLEND_ZERO;
		customRTBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		customRTBlendDesc.RenderTargetWriteMask = 0x0f;
	} else if (blendMode == GraphicsAPI::ADDITIVE) {
		customRTBlendDesc = {
			true, false,
			D3D12_BLEND_SRC_ALPHA, D3D12_BLEND_ONE, D3D12_BLEND_OP_ADD,
			D3D12_BLEND_ZERO, D3D12_BLEND_ONE, D3D12_BLEND_OP_ADD,
			D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL
		};
		customRTBlendDesc.BlendEnable = TRUE;
		customRTBlendDesc.SrcBlend = D3D12_BLEND_SRC_ALPHA;
		customRTBlendDesc.DestBlend = D3D12_BLEND_ONE;
		customRTBlendDesc.BlendOp = D3D12_BLEND_OP_ADD;
		customRTBlendDesc.SrcBlendAlpha = D3D12_BLEND_SRC_ALPHA;
		customRTBlendDesc.DestBlendAlpha = D3D12_BLEND_ONE;
		customRTBlendDesc.BlendOpAlpha = D3D12_BLEND_OP_ADD;
		customRTBlendDesc.RenderTargetWriteMask = 0x0f;
	}

	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++) { // TODO: change 1 to variable
		gpsd.BlendState.RenderTarget[i] = defaultRTdesc;
	}

	// gpsd.BlendState.IndependentBlendEnable = true; // What is this for again?
	gpsd.BlendState.RenderTarget[0] = customRTBlendDesc;

	// Specify depth stencil state descriptor.
	D3D12_DEPTH_STENCIL_DESC dsDesc{};
	dsDesc.DepthEnable = enableDepth;
	dsDesc.DepthWriteMask = (enableDepthWrite) ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
	dsDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	dsDesc.StencilEnable = FALSE;
	dsDesc.StencilReadMask = D3D12_DEFAULT_STENCIL_READ_MASK;
	dsDesc.StencilWriteMask = D3D12_DEFAULT_STENCIL_WRITE_MASK;
	const D3D12_DEPTH_STENCILOP_DESC defaultStencilOp = { D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_STENCIL_OP_KEEP, D3D12_COMPARISON_FUNC_ALWAYS };
	dsDesc.FrontFace = defaultStencilOp;
	dsDesc.BackFace = defaultStencilOp;

	gpsd.DepthStencilState = dsDesc;
	gpsd.DSVFormat = DXGI_FORMAT_D32_FLOAT;

	ThrowIfFailed(m_context->getDevice()->CreateGraphicsPipelineState(&gpsd, IID_PPV_ARGS(&m_pipelineState)));
}

void DX12ShaderPipeline::createComputePipelineState() {
	auto csD3DBlob = static_cast<ID3DBlob*>(csBlob);

	////// Pipeline State //////
	D3D12_COMPUTE_PIPELINE_STATE_DESC cpsd = {};

	// Specify pipeline stages
	cpsd.pRootSignature = m_context->getGlobalRootSignature();
	cpsd.CS.pShaderBytecode = reinterpret_cast<void*>(csD3DBlob->GetBufferPointer());;
	cpsd.CS.BytecodeLength = csD3DBlob->GetBufferSize();

	ThrowIfFailed(m_context->getDevice()->CreateComputePipelineState(&cpsd, IID_PPV_ARGS(&m_pipelineState)));
}