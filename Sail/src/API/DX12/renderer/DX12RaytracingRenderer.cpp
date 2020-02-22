#include "pch.h"
#include "DX12RaytracingRenderer.h"
#include "Sail/api/shader/PipelineStateObject.h"
#include "Sail/graphics/light/LightSetup.h"
#include "Sail/api/shader/Shader.h"
#include "Sail/Application.h"
#include "../DX12Utils.h"
#include "../shader/DX12PipelineStateObject.h"
#include "../resources/DescriptorHeap.h"
#include <unordered_set>
#include "../shader/DX12Shader.h"
#include "DX12DeferredRenderer.h"

DX12RaytracingRenderer::DX12RaytracingRenderer() {
	m_context = Application::getInstance()->getAPI<DX12API>();
	m_context->initCommand(m_command);
	m_command.list->SetName(L"Raytracing Renderer main command list");

	m_dxrBase = std::make_unique<DXRBase>("Basic");

	auto width = Application::getInstance()->getWindow()->getWindowWidth();
	auto height = Application::getInstance()->getWindow()->getWindowHeight();
	m_rtOutputTexture = std::unique_ptr<DX12RenderableTexture>(static_cast<DX12RenderableTexture*>(RenderableTexture::Create(width, height)));
}

DX12RaytracingRenderer::~DX12RaytracingRenderer() { }

void DX12RaytracingRenderer::begin(Camera* camera, Environment* environment) {
	Renderer::begin(camera, environment);
}

void* DX12RaytracingRenderer::present(Renderer::PresentFlag flags, void* skippedPrepCmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	ID3D12GraphicsCommandList4* cmdList = nullptr;
	if (flags & Renderer::PresentFlag::SkipPreparation) {
		if (skippedPrepCmdList)
			cmdList = static_cast<ID3D12GraphicsCommandList4*>(skippedPrepCmdList);
		else
			Logger::Error("DX12RaytracingRenderer present was called with skipPreparation flag but no cmdList was passed");
	}

	auto frameIndex = m_context->getFrameIndex();

	auto& allocator = m_command.allocators[frameIndex];
	cmdList = m_command.list.Get();

	// Reset allocators and lists for this frame
	allocator->Reset();
	cmdList->Reset(allocator.Get(), nullptr);
	
	// dispatch n stuff
	m_dxrBase->updateSceneData(camera, lightSetup);
	m_dxrBase->updateAccelerationStructures(commandQueue, cmdList);
	m_dxrBase->dispatch(m_rtOutputTexture.get(), cmdList);

	// Copy output to backbuffer

	cmdList->Close();
	m_context->getDirectQueue()->executeCommandLists({ cmdList });

	return cmdList;
}
