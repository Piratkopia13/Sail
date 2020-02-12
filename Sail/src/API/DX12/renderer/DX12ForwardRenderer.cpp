#include "pch.h"
#include "DX12ForwardRenderer.h"
#include "Sail/api/shader/PipelineStateObject.h"
#include "Sail/graphics/light/LightSetup.h"
#include "Sail/api/shader/Shader.h"
#include "Sail/Application.h"
#include "../DX12Utils.h"
#include "../shader/DX12PipelineStateObject.h"
#include "../resources/DescriptorHeap.h"
#include <unordered_set>
#include "../shader/DX12Shader.h"

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
	//std::unordered_set<DX12PipelineStateObject*> uniqueShaderPipelines;
	{
		SAIL_PROFILE_API_SPECIFIC_SCOPE("commandQueue loop");

		auto& resman = Application::getInstance()->getResourceManager();

		// TODO: Sort meshes according to shaderPipeline
		unsigned int totalInstances = commandQueue.size();
		for (RenderCommand& command : commandQueue) {
			DX12Shader* shader = static_cast<DX12Shader*>(command.mesh->getShader());
			//uniqueShaderPipelines.insert(shaderPipeline);

			// Make sure that constant buffers have a size that can allow the amount of meshes that will be rendered this frame
			shader->reserve(totalInstances);

			// Find a matching pipelineStateObject and bind it
			auto& pso = resman.getPSO(shader, command.mesh);
			pso.bind(cmdList.Get());
			//shaderPipeline->bind(command.mesh->getInputLayout(), cmdList.Get());

			shader->trySetCBufferVar("sys_mWorld", &glm::transpose(command.transform), sizeof(glm::mat4));
			shader->trySetCBufferVar("sys_mView", &camera->getViewMatrix(), sizeof(glm::mat4));
			shader->trySetCBufferVar("sys_mProjection", &camera->getProjMatrix(), sizeof(glm::mat4));
			shader->trySetCBufferVar("sys_mVP", &camera->getViewProjection(), sizeof(glm::mat4));
			shader->trySetCBufferVar("sys_cameraPos", &camera->getPosition(), sizeof(glm::vec3));

			if (lightSetup) {
				auto& [dlData, dlDataByteSize] = lightSetup->getDirLightData();
				auto& [plData, plDataByteSize] = lightSetup->getPointLightsData();
				shader->trySetCBufferVar("dirLight", dlData, dlDataByteSize);
				shader->trySetCBufferVar("pointLights", plData, plDataByteSize);
			}

			command.mesh->draw(*this, command.material, environment, cmdList.Get());
		}
	}

	// Reset pipelines so that they will re-bind the next frame
	/*for (auto* shaderPipeline : uniqueShaderPipelines) {
		shaderPipeline->unbind();
	}*/

	{
		SAIL_PROFILE_API_SPECIFIC_SCOPE("Execution");

		// Lastly - transition back buffer to present
		m_context->prepareToPresent(cmdList.Get());
		// Execute command list
		cmdList->Close();
		m_context->getDirectQueue()->executeCommandLists({ cmdList.Get() });
	}

}
