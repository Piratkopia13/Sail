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
#include "Sail/KeyCodes.h"

std::unique_ptr<DX12RenderableTexture> DX12RaytracingRenderer::sRTOutputTexture;

DX12RaytracingRenderer::DX12RaytracingRenderer() {
	EventSystem::getInstance()->subscribeToEvent(Event::WINDOW_RESIZE, this);

	m_context = Application::getInstance()->getAPI<DX12API>();
	m_context->initCommand(m_command);
	m_command.list->SetName(L"Raytracing Renderer main command list");

	m_dxrBase = std::make_unique<DXRHardShadows>();

	auto width = Application::getInstance()->getWindow()->getWindowWidth();
	auto height = Application::getInstance()->getWindow()->getWindowHeight();
	sRTOutputTexture = std::unique_ptr<DX12RenderableTexture>(static_cast<DX12RenderableTexture*>(RenderableTexture::Create(width, height, RenderableTexture::USAGE_GENERAL, "Raytracing output texture", ResourceFormat::R16G16B16A16_FLOAT)));
}

DX12RaytracingRenderer::~DX12RaytracingRenderer() {
	EventSystem::getInstance()->unsubscribeFromEvent(Event::WINDOW_RESIZE, this);
	m_context->waitForGPU();
	sRTOutputTexture.reset();
}

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

	if (!(flags & Renderer::PresentFlag::SkipPreparation)) {
		auto frameIndex = m_context->getFrameIndex();

		auto& allocator = m_command.allocators[frameIndex];
		cmdList = m_command.list.Get();

		// Reset allocators and lists for this frame
		allocator->Reset();
		cmdList->Reset(allocator.Get(), nullptr);
	}
	
	if (!(flags & Renderer::PresentFlag::SkipRendering)) {
		// dispatch n stuff
		m_dxrBase->updateSceneData(camera, lightSetup);
		//if (Input::IsKeyPressed(SAIL_KEY_L))
		m_dxrBase->updateAccelerationStructures(commandQueueCustom, cmdList);
		//if (Input::IsKeyPressed(SAIL_KEY_K))
		m_dxrBase->dispatch(sRTOutputTexture.get(), cmdList);
	}

	if (!(flags & Renderer::PresentFlag::SkipExecution)) {
		cmdList->Close();
		m_context->getDirectQueue()->executeCommandLists({ cmdList });
	}

	return cmdList;
}

DX12RenderableTexture* DX12RaytracingRenderer::GetOutputTexture() {
	return sRTOutputTexture.get();
}

bool DX12RaytracingRenderer::onEvent(Event& event) {
	auto resizeEvent = [&](WindowResizeEvent& event) {
		sRTOutputTexture->resize(event.getWidth(), event.getHeight());

		// Gbuffers will be resized inside DX12DeferredRenderer::onEvent, 
		// but since we have no control over if that method executes before 
		// this one we have to make sure that is has been done before calling 
		// dxrBase->recreateResources().
		// Calling resize on a texture which already has the given dimensions
		// will return without doing anything, making this safe.
		const auto& gbuffers = DX12DeferredRenderer::GetGBuffers();
		gbuffers.positions->resize(event.getWidth(), event.getHeight());
		gbuffers.normals->resize(event.getWidth(), event.getHeight());
		gbuffers.albedo->resize(event.getWidth(), event.getHeight());
		gbuffers.mrao->resize(event.getWidth(), event.getHeight());
		gbuffers.depth->resize(event.getWidth(), event.getHeight());

		// Tell dxrBase to update any references held to textures that how have resized (gbuffer inputs and rt output etc)
		m_dxrBase->recreateResources();

		return true;
	};
	EventHandler::HandleType<WindowResizeEvent>(event, resizeEvent);
	return true;
}
