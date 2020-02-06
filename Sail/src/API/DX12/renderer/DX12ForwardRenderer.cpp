#include "pch.h"
#include "DX12ForwardRenderer.h"
#include "Sail/api/shader/ShaderPipeline.h"
#include "Sail/graphics/light/LightSetup.h"
#include "Sail/graphics/shader/Shader.h"
#include "Sail/Application.h"
#include "../DX12Utils.h"
#include "../shader/DX12ShaderPipeline.h"
#include "../resources/DescriptorHeap.h"
#include <unordered_set>

Renderer* Renderer::Create(Renderer::Type type) {
	switch (type) {
	case FORWARD:
		return new DX12ForwardRenderer();
	default:
		Logger::Error("Tried to create a renderer of unknown or unimplemented type: " + std::to_string(type));
	}
	return nullptr;
}

DX12ForwardRenderer::DX12ForwardRenderer() {
	m_context = Application::getInstance()->getAPI<DX12API>();
	m_context->initCommand(m_command);
	m_command.list->SetName(L"Forward Renderer main command list");
}

DX12ForwardRenderer::~DX12ForwardRenderer() {

}

void DX12ForwardRenderer::present(RenderableTexture* output) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	assert(!output); // Render to texture is currently not implemented for DX12!

	auto frameIndex = m_context->getFrameIndex();

	auto& allocator = m_command.allocators[frameIndex];
	auto& cmdList = m_command.list;

	{
		SAIL_PROFILE_API_SPECIFIC_SCOPE("Preparation");

		// Reset allocators and lists for this frame
		allocator->Reset();
		cmdList->Reset(allocator.Get(), nullptr);

		// Bind the descriptor heap that will contain all SRVs for this frame
		m_context->getMainGPUDescriptorHeap()->bind(cmdList.Get());

		// Initialize resources
		// This executes texture mip generation
		m_context->initResources(cmdList.Get());

		// Transition back buffer to render target
		m_context->prepareToRender(cmdList.Get());

		m_context->renderToBackBuffer(cmdList.Get());
		cmdList->SetGraphicsRootSignature(m_context->getGlobalRootSignature());
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Bind mesh-common constant buffers (camera)
		// TODO: bind camera cbuffer here
		//cmdList->SetGraphicsRootConstantBufferView(GlobalRootParam::CBV_CAMERA, asdf);
	}

	// Keep track of unique shader pipelines used this frame
	std::unordered_set<DX12ShaderPipeline*> uniqueShaderPipelines;
	{
		SAIL_PROFILE_API_SPECIFIC_SCOPE("commandQueue loop");

		// TODO: Sort meshes according to material
		unsigned int totalInstances = commandQueue.size();
		for (RenderCommand& command : commandQueue) {
			DX12ShaderPipeline* shaderPipeline = static_cast<DX12ShaderPipeline*>(command.mesh->getShader()->getPipeline());
			uniqueShaderPipelines.insert(shaderPipeline);

			// Make sure that constant buffers have a size that can allow the amount of meshes that will be rendered this frame
			shaderPipeline->reserve(totalInstances);

			shaderPipeline->bind(cmdList.Get());

			shaderPipeline->setCBufferVar("sys_mWorld", &glm::transpose(command.transform), sizeof(glm::mat4));
			shaderPipeline->trySetCBufferVar("sys_mView", &camera->getViewMatrix(), sizeof(glm::mat4));
			shaderPipeline->trySetCBufferVar("sys_mProjection", &camera->getProjMatrix(), sizeof(glm::mat4));
			shaderPipeline->trySetCBufferVar("sys_mVP", &camera->getViewProjection(), sizeof(glm::mat4));
			shaderPipeline->setCBufferVar("sys_cameraPos", &camera->getPosition(), sizeof(glm::vec3));

			if (lightSetup) {
				auto& dlData = lightSetup->getDirLightData();
				auto& plData = lightSetup->getPointLightsData();
				shaderPipeline->setCBufferVar("dirLight", &dlData, sizeof(dlData));
				shaderPipeline->setCBufferVar("pointLights", &plData, sizeof(plData));
			}

			command.mesh->draw(*this, command.material, cmdList.Get());
		}
	}

	// Reset pipelines so that they will re-bind the next frame
	for (auto* shaderPipeline : uniqueShaderPipelines) {
		shaderPipeline->unbind();
	}

	{
		SAIL_PROFILE_API_SPECIFIC_SCOPE("Execution");

		// Lastly - transition back buffer to present
		m_context->prepareToPresent(cmdList.Get());
		// Execute command list
		cmdList->Close();
		m_context->getDirectQueue()->executeCommandLists({ cmdList.Get() });
	}

}
