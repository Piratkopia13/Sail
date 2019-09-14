#include "pch.h"
#include "DX12ComputeShaderDispatcher.h"
#include "Sail/Application.h"
#include "resources/DescriptorHeap.h"
#include "resources/DX12Texture.h"
#include "DX12Utils.h"

ComputeShaderDispatcher* ComputeShaderDispatcher::Create(Shader& computeShader) {
	return SAIL_NEW DX12ComputeShaderDispatcher(computeShader);
}

DX12ComputeShaderDispatcher::DX12ComputeShaderDispatcher(Shader& computeShader) 
	: ComputeShaderDispatcher(computeShader)
{
	assert(computeShader.getPipeline()->isComputeShader()); // Not a compute shader
	m_context = Application::getInstance()->getAPI<DX12API>();
}

DX12ComputeShaderDispatcher::~DX12ComputeShaderDispatcher() {

}

void DX12ComputeShaderDispatcher::dispatch(void* cmdList) {
	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);
	
	m_context->getComputeGPUDescriptorHeap()->bind(dxCmdList);
	dxCmdList->SetComputeRootSignature(m_context->getGlobalRootSignature());
 
	const auto& settings = computeShader.getComputeSettings();

	if (settings->usesCBV_SRV_UAV)
		dxCmdList->SetComputeRootDescriptorTable(m_context->getRootIndexFromRegister("t0"), m_context->getComputeGPUDescriptorHeap()->getGPUDescriptorHandleForIndex(0));

	// Resize output textures
	for (unsigned int i = 0; i < settings->numOutputTextures; i++) {
		const auto& tex = computeShader.getComputeOutputForIndex(*computeShader.getComputeOutput(), i);
		tex->resize(input->outputWidth, input->outputHeight);
	}

	// Bind input textures
	for (unsigned int i = 0; i < settings->numInputTextures; i++) {
		auto& tex = computeShader.getComputeInputForIndex(*input, i);

		computeShader.getPipeline()->setTexture2D(tex.first, tex.second, dxCmdList);
		static_cast<DX12Texture*>(tex.second)->transitionStateTo(dxCmdList, D3D12_RESOURCE_STATE_GENERIC_READ);
		// Skip the next 2 heap slots to match layout
		m_context->getComputeGPUDescriptorHeap()->getNextCPUDescriptorHandle();
		m_context->getComputeGPUDescriptorHeap()->getNextCPUDescriptorHandle();
	}
	// Bind output resources
	computeShader.getPipeline()->bind(dxCmdList);

	computeShader.getPipeline()->dispatch(input->threadGroupCountX, input->threadGroupCountY, input->threadGroupCountZ, dxCmdList);

	//DX12Utils::SetResourceTransitionBarrier(dxCmdList, static_cast<DX12Texture*>(mat->getTexture(0))->getBuffer(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
}

Shader::ComputeShaderOutput& DX12ComputeShaderDispatcher::getOutput() {
	return *computeShader.getComputeOutput();
}
