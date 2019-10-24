#include "pch.h"
#include "DX12ScreenSpaceRenderer.h"
#include "Sail/Application.h"
#include "API/DX12/resources/DX12Texture.h"
#include "API/DX12/shader/DX12ShaderPipeline.h"
#include "API/DX12/DX12Mesh.h"
#include "Sail/graphics/geometry/PBRMaterial.h"
#include "Sail/graphics/shader/Shader.h"

DX12ScreenSpaceRenderer::DX12ScreenSpaceRenderer() {
	Application* app = Application::getInstance();
	m_context = app->getAPI<DX12API>();

	m_context->initCommand(m_command);
	std::wstring name = L"ScreenSpace Renderer main command list";
	m_command.list->SetName(name.c_str());
	
	auto windowWidth = app->getWindow()->getWindowWidth();
	auto windowHeight = app->getWindow()->getWindowHeight();
}

DX12ScreenSpaceRenderer::~DX12ScreenSpaceRenderer() {}

void DX12ScreenSpaceRenderer::begin(Camera* camera) {}

void DX12ScreenSpaceRenderer::present(PostProcessPipeline* postProcessPipeline, RenderableTexture* output) {
	assert(!output); // Render to texture is currently not implemented for DX12!

	auto frameIndex = m_context->getSwapIndex();

	auto& allocator = m_command.allocators[m_context->getFrameIndex()];
	auto& cmdList = m_command.list;

	// Reset allocators and lists for this frame
	allocator->Reset();
	cmdList->Reset(allocator.Get(), nullptr);

	m_context->renderToBackBuffer(cmdList.Get());
	m_context->prepareToRender(cmdList.Get());
		
	// Init all textures - this needs to be done on ONE thread
	// TODO: optimize!
	int meshIndex = 0;
	for (auto& renderCommand : commandQueue) {
		for (int i = 0; i < 3; i++) {
			if (renderCommand.type != RENDER_COMMAND_TYPE_MODEL) {
				continue;
			}
			auto* tex = static_cast<DX12Texture*>(renderCommand.model.mesh->getMaterial()->getTexture(i));
			if (tex && !tex->hasBeenInitialized()) {
				tex->initBuffers(cmdList.Get(), meshIndex);
				meshIndex++;
			}
		}
	}

	cmdList->SetGraphicsRootSignature(m_context->getGlobalRootSignature());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Bind the descriptor heap that will contain all SRVs for this frame
	m_context->getMainGPUDescriptorHeap()->bind(cmdList.Get());

	meshIndex = 0;
	for (auto& command : commandQueue) {
		DX12ShaderPipeline* shaderPipeline = static_cast<DX12ShaderPipeline*>(command.model.mesh->getMaterial()->getShader()->getPipeline());

		shaderPipeline->checkBufferSizes(commandQueue.size()); //Temp fix to expand constant buffers if the scene contain to many objects
		shaderPipeline->bind_new(cmdList.Get(), meshIndex);

		// Used in most shaders
		static_cast<DX12Mesh*>(command.model.mesh)->draw_new(*this, cmdList.Get(), meshIndex);
		meshIndex++;
	}

	// Lastly - transition back buffer to present
	m_context->prepareToPresent(cmdList.Get());
	// Close command list
	cmdList->Close();

	m_context->executeCommandLists({m_command.list.Get()});
}

bool DX12ScreenSpaceRenderer::onEvent(Event& event) {
	return false;
}
