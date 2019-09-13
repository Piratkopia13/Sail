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
	m_context = Application::getInstance()->getAPI<DX12API>();

	if (!m_dxilCompiler) {
		m_dxilCompiler = std::make_unique<DXILShaderCompiler>();
		m_dxilCompiler->init();
	}
}

DX12ShaderPipeline::~DX12ShaderPipeline() {
	m_context->waitForGPU();
}

void DX12ShaderPipeline::bind(void* cmdList) {
	if (!m_pipelineState)
		Logger::Error("Tried to bind DX12PipelineState before the DirectX PipelineStateObject has been created!");
	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);
	ShaderPipeline::bind(cmdList);
	for (auto& it : parsedData.renderableTextures) {
		static_cast<DX12RenderableTexture*>(it.renderableTexture.get())->bind(dxCmdList);
	}
	dxCmdList->SetPipelineState(m_pipelineState.Get());
}

void DX12ShaderPipeline::dispatch(unsigned int threadGroupCountX, unsigned int threadGroupCountY, unsigned int threadGroupCountZ, void* cmdList) {
	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);
	dxCmdList->Dispatch(threadGroupCountX, threadGroupCountY, threadGroupCountZ);
}

void* DX12ShaderPipeline::compileShader(const std::string& source, const std::string& filepath, ShaderComponent::BIND_SHADER shaderType) {

	// TODO: make this work
//	DXILShaderCompiler::Desc shaderDesc;
//	
//	switch (shaderType) {
//	case ShaderComponent::VS:
//		shaderDesc.entryPoint = L"VSMain";
//		shaderDesc.targetProfile = L"vs_6_0";
//		break;
//	case ShaderComponent::PS:
//		shaderDesc.entryPoint = L"PSMain";
//		shaderDesc.targetProfile = L"ps_6_0";
//		break;
//	case ShaderComponent::GS:
//		shaderDesc.entryPoint = L"GSMain";
//		shaderDesc.targetProfile = L"gs_6_0";
//		break;
//	case ShaderComponent::CS:
//		shaderDesc.entryPoint = L"CSMain";
//		shaderDesc.targetProfile = L"cs_6_0";
//		break;
//	case ShaderComponent::DS:
//		shaderDesc.entryPoint = L"DSMain";
//		shaderDesc.targetProfile = L"ds_6_0";
//		break;
//	case ShaderComponent::HS:
//		shaderDesc.entryPoint = L"HSMain";
//		shaderDesc.targetProfile = L"hs_6_0";
//		break;
//	}
//	
//#ifdef _DEBUG
//	shaderDesc.compileArguments.push_back(L"/Zi"); // Debug info
//#endif
//	shaderDesc.compileArguments.push_back(L"/Gis"); // Declare all variables and values as precise
//	shaderDesc.source = source.c_str();
//	shaderDesc.sourceSize = source.length();
//	auto wfilepath = std::wstring(filepath.begin(), filepath.end());
//	shaderDesc.filePath = wfilepath.c_str();
//
//	IDxcBlob* pShaders = nullptr;
//	ThrowIfFailed(m_dxilCompiler->compile(&shaderDesc, &pShaders));

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

	return pShaders;

}

void DX12ShaderPipeline::setTexture2D(const std::string& name, Texture* texture, void* cmdList) {
	DX12Texture* dxTexture = static_cast<DX12Texture*>(texture);
	if (!dxTexture->hasBeenInitialized())
		dxTexture->initBuffers(static_cast<ID3D12GraphicsCommandList4*>(cmdList));

	// Copy texture SRVs to the gpu heap
	m_context->getDevice()->CopyDescriptorsSimple(1, m_context->getMainGPUDescriptorHeap()->getNextCPUDescriptorHandle(), dxTexture->getCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
}

void DX12ShaderPipeline::setResourceHeapMeshIndex(unsigned int index) {
	for (auto& it : parsedData.cBuffers) {
		static_cast<ShaderComponent::DX12ConstantBuffer*>(it.cBuffer.get())->setResourceHeapMeshIndex(index);
	}
}

void DX12ShaderPipeline::createGraphicsPipelineState() {
	auto vsD3DBlob = static_cast<ID3DBlob*>(vsBlob);
	auto psD3DBlob = static_cast<ID3DBlob*>(psBlob);
	auto gsD3DBlob = static_cast<ID3DBlob*>(gsBlob);
	auto dsD3DBlob = static_cast<ID3DBlob*>(dsBlob);
	auto hsD3DBlob = static_cast<ID3DBlob*>(hsBlob);
	auto csD3DBlob = static_cast<ID3DBlob*>(csBlob);
	
	////// Pipline State //////
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
	gpsd.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	gpsd.NumRenderTargets = 1;

	gpsd.SampleDesc.Count = 1;
	gpsd.SampleDesc.Quality = 0;
	gpsd.SampleMask = UINT_MAX;

	// Specify rasterizer behaviour
	// TODO: get these from DX12API
	gpsd.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;

	//Specify blend descriptions.
	D3D12_RENDER_TARGET_BLEND_DESC defaultRTdesc = {
		false, false,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
		D3D12_LOGIC_OP_NOOP, D3D12_COLOR_WRITE_ENABLE_ALL
	};
	for (UINT i = 0; i < D3D12_SIMULTANEOUS_RENDER_TARGET_COUNT; i++)
		gpsd.BlendState.RenderTarget[i] = defaultRTdesc;

	// Specify depth stencil state descriptor.
	D3D12_DEPTH_STENCIL_DESC dsDesc{};
	dsDesc.DepthEnable = TRUE;
	dsDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
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

	////// Pipline State //////
	D3D12_COMPUTE_PIPELINE_STATE_DESC cpsd = {};

	// Specify pipeline stages
	cpsd.pRootSignature = m_context->getGlobalRootSignature();
	cpsd.CS.pShaderBytecode = reinterpret_cast<void*>(csD3DBlob->GetBufferPointer());;
	cpsd.CS.BytecodeLength = csD3DBlob->GetBufferSize();

	ThrowIfFailed(m_context->getDevice()->CreateComputePipelineState(&cpsd, IID_PPV_ARGS(&m_pipelineState)));
}

void DX12ShaderPipeline::compile() {
	ShaderPipeline::compile();
}

void DX12ShaderPipeline::finish() {

	// Create a compute pipeline state if it has a compute shader
	if (csBlob) {
		createComputePipelineState();
	} else {
		createGraphicsPipelineState();
	}

}
