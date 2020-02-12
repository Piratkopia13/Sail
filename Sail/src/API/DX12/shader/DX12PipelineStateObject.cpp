#include "pch.h"
#include "DX12PipelineStateObject.h"
#include "Sail/Application.h"
#include "DX12InputLayout.h"
#include "../DX12API.h"
#include "Sail/api/shader/Shader.h"

std::unique_ptr<DXILShaderCompiler> DX12PipelineStateObject::m_dxilCompiler = nullptr;

PipelineStateObject* PipelineStateObject::Create(Shader* shader, unsigned int attributesHash) {
	return SAIL_NEW DX12PipelineStateObject(shader, attributesHash);
}

DX12PipelineStateObject::DX12PipelineStateObject(Shader* shader, unsigned int attributesHash)
	: PipelineStateObject(shader, attributesHash)
{
	m_context = Application::getInstance()->getAPI<DX12API>();

	if (!m_dxilCompiler) {
		m_dxilCompiler = std::make_unique<DXILShaderCompiler>();
		m_dxilCompiler->init();
	}

	// Create a compute pipeline state if it has a compute shader
	if (shader->isComputeShader()) {
		createComputePipelineState();
	} else {
		createGraphicsPipelineState();
	}
}

bool DX12PipelineStateObject::bind(void* cmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	if (!m_pipelineState)
		Logger::Error("Tried to bind DX12PipelineState before the DirectX PipelineStateObject has been created!");
	
	// TODO: This returns false if pipeline is already bound, maybe do something with that
	bindInternal(cmdList, true);
	
	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);
	dxCmdList->SetPipelineState(m_pipelineState.Get());

	return true;
}

void DX12PipelineStateObject::setRenderTargetFormat(unsigned int rtIndex, DXGI_FORMAT format) {
	m_rtFormats[rtIndex] = format;
}

void DX12PipelineStateObject::createGraphicsPipelineState() {
	auto vsD3DBlob = static_cast<ID3DBlob*>(shader->getVsBlob());
	auto psD3DBlob = static_cast<ID3DBlob*>(shader->getPsBlob());
	auto gsD3DBlob = static_cast<ID3DBlob*>(shader->getGsBlob());
	auto dsD3DBlob = static_cast<ID3DBlob*>(shader->getDsBlob());
	auto hsD3DBlob = static_cast<ID3DBlob*>(shader->getHsBlob());

	////// Pipeline State //////
	D3D12_GRAPHICS_PIPELINE_STATE_DESC gpsd = {};

	// Specify pipeline stages
	gpsd.pRootSignature = m_context->getGlobalRootSignature();
	gpsd.InputLayout = static_cast<DX12InputLayout*>(inputLayout.get())->getDesc();
	gpsd.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	if (vsD3DBlob) {
		gpsd.VS.pShaderBytecode = reinterpret_cast<void*>(vsD3DBlob->GetBufferPointer());
		gpsd.VS.BytecodeLength = vsD3DBlob->GetBufferSize();
	}
	if (psD3DBlob) {
		gpsd.PS.pShaderBytecode = reinterpret_cast<void*>(psD3DBlob->GetBufferPointer());
		gpsd.PS.BytecodeLength = psD3DBlob->GetBufferSize();
	}
	if (gsD3DBlob) {
		gpsd.GS.pShaderBytecode = reinterpret_cast<void*>(gsD3DBlob->GetBufferPointer());
		gpsd.GS.BytecodeLength = gsD3DBlob->GetBufferSize();
	}
	if (dsD3DBlob) {
		gpsd.DS.pShaderBytecode = reinterpret_cast<void*>(dsD3DBlob->GetBufferPointer());
		gpsd.DS.BytecodeLength = dsD3DBlob->GetBufferSize();
	}
	if (hsD3DBlob) {
		gpsd.HS.pShaderBytecode = reinterpret_cast<void*>(hsD3DBlob->GetBufferPointer());
		gpsd.HS.BytecodeLength = hsD3DBlob->GetBufferSize();
	}

	// Specify render target and depthstencil usage
	for (unsigned int i = 0; i < settings.numRenderTargets; i++) {
		if (m_rtFormats.find(i) != m_rtFormats.end()) {
			gpsd.RTVFormats[i] = m_rtFormats[i];
		} else {
			gpsd.RTVFormats[i] = DXGI_FORMAT_R8G8B8A8_UNORM;
		}
	}
	gpsd.NumRenderTargets = settings.numRenderTargets;

	gpsd.SampleDesc.Count = 1;
	gpsd.SampleDesc.Quality = 0;
	gpsd.SampleMask = UINT_MAX;

	// Specify rasterizer behaviour
	if (settings.wireframe) {
		gpsd.RasterizerState.FillMode = D3D12_FILL_MODE_WIREFRAME;
	} else {
		gpsd.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
	}

	if (settings.cullMode == GraphicsAPI::Culling::NO_CULLING) {
		gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
	} else if (settings.cullMode == GraphicsAPI::Culling::FRONTFACE) {
		gpsd.RasterizerState.CullMode = D3D12_CULL_MODE_FRONT;
	} else if (settings.cullMode == GraphicsAPI::Culling::BACKFACE) {
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
	if (settings.blendMode == GraphicsAPI::ALPHA) {
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
	} else if (settings.blendMode == GraphicsAPI::ADDITIVE) {
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
	dsDesc.DepthEnable = settings.depthMask != GraphicsAPI::BUFFER_DISABLED;
	dsDesc.DepthWriteMask = (settings.depthMask != GraphicsAPI::WRITE_MASK) ? D3D12_DEPTH_WRITE_MASK_ALL : D3D12_DEPTH_WRITE_MASK_ZERO;
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

void DX12PipelineStateObject::createComputePipelineState() {
	auto csD3DBlob = static_cast<ID3DBlob*>(shader->getCsBlob());

	////// Pipeline State //////
	D3D12_COMPUTE_PIPELINE_STATE_DESC cpsd = {};

	// Specify pipeline stages
	cpsd.pRootSignature = m_context->getGlobalRootSignature();
	cpsd.CS.pShaderBytecode = reinterpret_cast<void*>(csD3DBlob->GetBufferPointer());;
	cpsd.CS.BytecodeLength = csD3DBlob->GetBufferSize();

	ThrowIfFailed(m_context->getDevice()->CreateComputePipelineState(&cpsd, IID_PPV_ARGS(&m_pipelineState)));
}