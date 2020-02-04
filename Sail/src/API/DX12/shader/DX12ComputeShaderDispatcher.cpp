#include "pch.h"
#include "DX12ComputeShaderDispatcher.h"
#include "Sail/Application.h"
#include "../resources/DescriptorHeap.h"
#include "../resources/DX12Texture.h"
#include "../DX12Utils.h"
#include "API/DX12/shader/DX12ShaderPipeline.h"

ComputeShaderDispatcher* ComputeShaderDispatcher::Create() {
	return SAIL_NEW DX12ComputeShaderDispatcher();
}

DX12ComputeShaderDispatcher::DX12ComputeShaderDispatcher() {
	m_context = Application::getInstance()->getAPI<DX12API>();
}

DX12ComputeShaderDispatcher::~DX12ComputeShaderDispatcher() { }

void DX12ComputeShaderDispatcher::begin(void* cmdList) {
	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);

	m_context->getMainGPUDescriptorHeap()->bind(dxCmdList);
	dxCmdList->SetComputeRootSignature(m_context->getGlobalRootSignature());
}

void DX12ComputeShaderDispatcher::dispatch(Shader& computeShader, const glm::vec3& threadGroupCount, void* cmdList) {
	assert(computeShader.getPipeline()->isComputeShader()); // Not a compute shader
	auto* dxShaderPipeline = static_cast<DX12ShaderPipeline*>(computeShader.getPipeline());
	

	auto* dxCmdList = static_cast<ID3D12GraphicsCommandList4*>(cmdList);
	const auto& settings = computeShader.getComputeSettings();

	//// Resize output textures
	//for (unsigned int i = 0; i < settings->numOutputTextures; i++) {
	//	const auto& tex = computeShader.getComputeOutputForIndex(*computeShader.getComputeOutput(), i);
	//	tex->resize(input.outputWidth, input.outputHeight);
	//}

	//// Bind input textures
	//for (unsigned int i = 0; i < settings->numInputTextures; i++) {
	//	auto& tex = computeShader.getComputeInputForIndex(input, i);
	//	DX12ATexture* texture = (DX12ATexture*)tex.second;
	//	
	//	if (texture->isRenderable()) {
	//		dxShaderPipeline->setTexture2D(tex.first, (RenderableTexture*)texture, dxCmdList);
	//	} else {
	//		dxShaderPipeline->setTexture2D(tex.first, (Texture*)texture, dxCmdList);
	//	}
	//}
	//if (settings->numInputTextures > 0) {
	//	// Skip the next x heap slots to match root signature layout
	//	// TODO: read this from the root signature, currently it will crash if the root signature changes num srv descriptors
	//	m_context->getMainGPUDescriptorHeap()->getAndStepIndex(10 - settings->numInputTextures);
	//}
	// Bind output resources
	dxShaderPipeline->bind(dxCmdList);

	assert(threadGroupCount.x <= D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION && "There are too many x threads!!");
	assert(threadGroupCount.y <= D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION && "There are too many y threads!!");
	assert(threadGroupCount.z <= D3D12_CS_DISPATCH_MAX_THREAD_GROUPS_PER_DIMENSION && "There are too many z threads!!");

	dxCmdList->Dispatch(threadGroupCount.x, threadGroupCount.y, threadGroupCount.z);

	dxShaderPipeline->instanceFinished();

	//return *computeShader.getComputeOutput();
}