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
#include "../dxr/impl/DXRHardShadows.h"
#include "../dxr/impl/DXRReflections.h"

std::vector<DX12RenderableTexture*> DX12RaytracingRenderer::sRTOutputTextures;

DX12RaytracingRenderer::DX12RaytracingRenderer() {
	EventSystem::getInstance()->subscribeToEvent(Event::WINDOW_RESIZE, this);

	m_context = Application::getInstance()->getAPI<DX12API>();
	m_context->initCommand(m_command);
	m_command.list->SetName(L"Raytracing Renderer main command list");

	//m_dxrBase = std::make_unique<DXRHardShadows>();
	m_dxrBase = std::make_unique<DXRReflections>();

	auto width = Application::getInstance()->getWindow()->getWindowWidth();
	auto height = Application::getInstance()->getWindow()->getWindowHeight();

	unsigned int numOutputTextures = 4;
	sRTOutputTextures.resize(numOutputTextures);
	for (unsigned int i = 0; i < numOutputTextures; i++)
		sRTOutputTextures[i] = SAIL_NEW DX12RenderableTexture(1, width, height, ResourceFormat::R16G16B16A16_FLOAT, 
			false, false, { 0.f, 0.f, 0.f, 0.f }, true, 0U, 0U, "Raytracing output texture " + std::to_string(i), 1);
}

DX12RaytracingRenderer::~DX12RaytracingRenderer() {
	EventSystem::getInstance()->unsubscribeFromEvent(Event::WINDOW_RESIZE, this);
	for (auto& tex : sRTOutputTextures)
		delete tex;
	sRTOutputTextures.clear();
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
		m_dxrBase->updateAccelerationStructures(commandQueue, cmdList);
		//if (Input::IsKeyPressed(SAIL_KEY_K))
		const auto& outputs = sRTOutputTextures.data();
		m_dxrBase->dispatch(outputs, sRTOutputTextures.size(), cmdList);
	}

	if (!(flags & Renderer::PresentFlag::SkipExecution)) {
		cmdList->Close();
		m_context->getDirectQueue()->executeCommandLists({ cmdList });
	}

	return cmdList;
}

const std::vector<DX12RenderableTexture*>& DX12RaytracingRenderer::GetOutputTextures() {
	return sRTOutputTextures;
}

bool DX12RaytracingRenderer::onEvent(Event& event) {
	auto resizeEvent = [&](WindowResizeEvent& event) {
		for (auto& tex : sRTOutputTextures)
			tex->resize(event.getWidth(), event.getHeight());

		// Gbuffers will be resized inside DX12DeferredRenderer::onEvent, 
		// but since we have no control over if that method executes before 
		// this one we have to make sure that is has been done before calling 
		// dxrBase->recreateResources().
		// Calling resize on a texture which already has the given dimensions
		// will return without doing anything, making this safe.
		const auto& gbuffers = DX12DeferredRenderer::GetGBuffers();
		for (unsigned i = 0; i < DX12DeferredRenderer::NUM_GBUFFERS; i++) {
			gbuffers[i]->resize(event.getWidth(), event.getHeight());
		}

		// Tell dxrBase to update any references held to textures that how have resized (gbuffer inputs and rt output etc)
		m_dxrBase->recreateResources();

		return true;
	};
	EventHandler::HandleType<WindowResizeEvent>(event, resizeEvent);
	return true;
}
