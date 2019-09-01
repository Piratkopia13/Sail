#include "pch.h"
#include "DX12RaytracingRenderer.h"
#include "Sail/Application.h"
#include "../DX12Utils.h"

// Current goal is to make this render a fully raytraced image of all geometry (without materials) within a scene

DX12RaytracingRenderer::DX12RaytracingRenderer() 
	: m_dxr("Basic")
{
	m_context = Application::getInstance()->getAPI<DX12API>();
	m_context->initCommand(m_command);
	m_command.list->SetName(L"Raytracing Renderer main command list");
}

DX12RaytracingRenderer::~DX12RaytracingRenderer() {

}

void DX12RaytracingRenderer::present(RenderableTexture* output) {

	auto frameIndex = m_context->getFrameIndex();

	auto& allocator = m_command.allocators[frameIndex];
	auto& cmdList = m_command.list;

	// Reset allocators and lists for this frame
	allocator->Reset();
	cmdList->Reset(allocator.Get(), nullptr);

	m_dxr.updateAccelerationStructures(commandQueue, cmdList.Get());
	m_dxr.updateCamera(*camera);
	auto* dxrOutput = m_dxr.dispatch(cmdList.Get());

	// TODO: move this to a graphics queue when current cmdList is executed on the compute queue

	DX12Utils::SetResourceTransitionBarrier(cmdList.Get(), dxrOutput, D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE);
	auto* renderTarget = m_context->getCurrentRenderTargetResource();
	DX12Utils::SetResourceTransitionBarrier(cmdList.Get(), renderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdList->CopyResource(renderTarget, dxrOutput);
	DX12Utils::SetResourceTransitionBarrier(cmdList.Get(), renderTarget, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);

	// Execute command list
	cmdList->Close();
	m_context->executeCommandLists({ cmdList.Get() });


}
