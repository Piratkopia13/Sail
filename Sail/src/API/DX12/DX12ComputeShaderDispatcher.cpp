#include "pch.h"
#include "DX12ComputeShaderDispatcher.h"
#include "Sail/Application.h"
#include "resources/DescriptorHeap.h"
#include "resources/DX12Texture.h"
#include "DX12Utils.h"

ComputeShaderDispatcher* ComputeShaderDispatcher::Create() {
	return SAIL_NEW DX12ComputeShaderDispatcher();
}

DX12ComputeShaderDispatcher::DX12ComputeShaderDispatcher() {
	m_context = Application::getInstance()->getAPI<DX12API>();
}

DX12ComputeShaderDispatcher::~DX12ComputeShaderDispatcher() { }

void DX12ComputeShaderDispatcher::begin(void* cmdList) {
	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);

	m_context->getComputeGPUDescriptorHeap()->bind(dxCmdList);
	dxCmdList->SetComputeRootSignature(m_context->getGlobalRootSignature());
}

Shader::ComputeShaderOutput& DX12ComputeShaderDispatcher::dispatch(Shader& computeShader, Shader::ComputeShaderInput& input, void* cmdList) {
	assert(computeShader.getPipeline()->isComputeShader()); // Not a compute shader
	
	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);
	const auto& settings = computeShader.getComputeSettings();

	if (settings->usesCBV_SRV_UAV)
		dxCmdList->SetComputeRootDescriptorTable(m_context->getRootIndexFromRegister("t0"), m_context->getComputeGPUDescriptorHeap()->getCurentGPUDescriptorHandle());

	// Resize output textures
	for (unsigned int i = 0; i < settings->numOutputTextures; i++) {
		const auto& tex = computeShader.getComputeOutputForIndex(*computeShader.getComputeOutput(), i);
		tex->resize(input.outputWidth, input.outputHeight);
	}

	// Bind input textures
	for (unsigned int i = 0; i < settings->numInputTextures; i++) {
		auto& tex = computeShader.getComputeInputForIndex(input, i);
		DX12ATexture* texture = static_cast<DX12ATexture*>(tex.second);
		
		if (texture->isRenderable()) {
			computeShader.getPipeline()->setTexture2D(tex.first, (RenderableTexture*)texture, dxCmdList);
		} else {
			computeShader.getPipeline()->setTexture2D(tex.first, (Texture*)texture, dxCmdList);
		}
		// Skip the next 2 heap slots to match root signature layout
		// TODO: read this from the root signature, currently it will crash if the root signature changes num srv descriptors
		m_context->getComputeGPUDescriptorHeap()->getNextCPUDescriptorHandle();
		m_context->getComputeGPUDescriptorHeap()->getNextCPUDescriptorHandle();
	}
	// Bind output resources
	computeShader.getPipeline()->bind(dxCmdList);

	computeShader.getPipeline()->dispatch(input.threadGroupCountX, input.threadGroupCountY, input.threadGroupCountZ, dxCmdList);

	return *computeShader.getComputeOutput();
}
