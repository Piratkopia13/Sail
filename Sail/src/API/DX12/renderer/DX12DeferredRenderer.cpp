#include "pch.h"
#include "DX12DeferredRenderer.h"
#include "Sail/api/shader/PipelineStateObject.h"
#include "Sail/graphics/light/LightSetup.h"
#include "Sail/api/shader/Shader.h"
#include "Sail/Application.h"
#include "../DX12Utils.h"
#include "../shader/DX12PipelineStateObject.h"
#include "../resources/DescriptorHeap.h"
#include <unordered_set>
#include "../shader/DX12Shader.h"
#include "Sail/graphics/geometry/factory/ScreenQuadModel.h"
#include "Sail/graphics/Environment.h"

DX12DeferredRenderer::DX12DeferredRenderer() {
	auto* app = Application::getInstance();
	m_context = app->getAPI<DX12API>();
	m_context->initCommand(m_command);
	m_command.list->SetName(L"Deferred Renderer main command list");

	auto windowWidth = app->getWindow()->getWindowWidth();
	auto windowHeight = app->getWindow()->getWindowHeight();

	for (int i = 0; i < NUM_GBUFFERS; i++) {
		m_gbufferTextures[i] = std::unique_ptr<DX12RenderableTexture>(static_cast<DX12RenderableTexture*>(
			RenderableTexture::Create(windowWidth, windowHeight, "GBuffer renderer output " + std::to_string(i), (i < 2) ? ResourceFormat::R16G16B16A16_FLOAT : ResourceFormat::R8G8B8A8, (i == 0))));
	}

	auto& shader = Application::getInstance()->getResourceManager().getShaderSet(Shaders::DeferredShadingPassShader);
	m_screenQuadModel = ModelFactory::ScreenQuadModel::Create(&shader);
}

DX12DeferredRenderer::~DX12DeferredRenderer() {

}

void DX12DeferredRenderer::present(RenderableTexture* output) {
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

		//m_context->renderToBackBuffer(cmdList.Get());
		// Bind gbuffer RTV and DSV
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[NUM_GBUFFERS];
		for (int i = 0; i < NUM_GBUFFERS; i++) {
			rtvHandles[i] = m_gbufferTextures[i]->getRtvCDH();
		}
		cmdList->OMSetRenderTargets(NUM_GBUFFERS, rtvHandles, false, &m_gbufferTextures[0]->getDsvCDH());
		// Transition gbuffers to render target
		for (int i = 0; i < NUM_GBUFFERS; i++) {
			// TODO: transition in batch
			m_gbufferTextures[i]->transitionStateTo(cmdList.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
			m_gbufferTextures[i]->clear({ 0.01f, 0.01f, 0.01f, 1.0f }, cmdList.Get());
		}
		cmdList->RSSetViewports(1, m_context->getViewport());
		cmdList->RSSetScissorRects(1, m_context->getScissorRect());

		cmdList->SetGraphicsRootSignature(m_context->getGlobalRootSignature());
		cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		// Bind mesh-common constant buffers (camera)
		// TODO: bind camera cbuffer here
		//cmdList->SetGraphicsRootConstantBufferView(GlobalRootParam::CBV_CAMERA, asdf);
	}

	auto& resman = Application::getInstance()->getResourceManager();
	{
		SAIL_PROFILE_API_SPECIFIC_SCOPE("commandQueue loop");


		// TODO: Sort meshes according to shaderPipeline
		unsigned int totalInstances = commandQueue.size();
		for (RenderCommand& command : commandQueue) {
			DX12Shader* shader = static_cast<DX12Shader*>(command.shader);
			//uniqueShaderPipelines.insert(shaderPipeline);

			// Make sure that constant buffers have a size that can allow the amount of meshes that will be rendered this frame
			shader->reserve(totalInstances);

			// Find a matching pipelineStateObject and bind it
			auto& pso = resman.getPSO(shader, command.mesh);
			pso.bind(cmdList.Get());

			shader->trySetCBufferVar("sys_mWorld", &glm::transpose(command.transform), sizeof(glm::mat4));
			shader->trySetCBufferVar("sys_mView", &camera->getViewMatrix(), sizeof(glm::mat4));
			shader->trySetCBufferVar("sys_mProjection", &camera->getProjMatrix(), sizeof(glm::mat4));
			shader->trySetCBufferVar("sys_mVP", &camera->getViewProjection(), sizeof(glm::mat4));

			command.mesh->draw(*this, command.material, shader, environment, cmdList.Get());
		}
	}
	
	// Shading pass
	{
		// Transition back buffer to render target
		m_context->prepareToRender(cmdList.Get());

		// Set back buffer as render target
		m_context->renderToBackBuffer(cmdList.Get());

		// Transition gbuffers to pixel shader resources
		for (int i = 0; i < NUM_GBUFFERS; i++)
			m_gbufferTextures[i]->transitionStateTo(cmdList.Get(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); // TODO: transition in batch

		auto* shader = &Application::getInstance()->getResourceManager().getShaderSet(Shaders::DeferredShadingPassShader);
		auto* mesh = m_screenQuadModel->getMesh(0);

		// Find a matching pipelineStateObject and bind it
		auto& pso = resman.getPSO(shader, mesh);
		pso.bind(cmdList.Get());

		shader->trySetCBufferVar("sys_mViewInv", &glm::inverse(camera->getViewMatrix()), sizeof(glm::mat4));
		shader->trySetCBufferVar("sys_cameraPos", &camera->getPosition(), sizeof(glm::vec3));

		if (lightSetup) {
			auto& [dlData, dlDataByteSize] = lightSetup->getDirLightData();
			auto& [plData, plDataByteSize] = lightSetup->getPointLightsData();
			shader->trySetCBufferVar("dirLight", dlData, dlDataByteSize);
			shader->trySetCBufferVar("pointLights", plData, plDataByteSize);
		}

		auto materialFunc = [&](Shader* shader, Environment* environment, void* cmdList) {
			auto* brdfLutTexture = &Application::getInstance()->getResourceManager().getTexture("pbr/brdfLUT.tga");
			// Bind environment textures
			shader->setTexture("sys_texBrdfLUT", brdfLutTexture, cmdList);
			shader->setTexture("irradianceMap", environment->getIrradianceTexture(), cmdList);
			shader->setTexture("radianceMap", environment->getRadianceTexture(), cmdList);

			// Bind GBuffer textures
			shader->setRenderableTexture("def_positions", m_gbufferTextures[0].get(), cmdList);
			shader->setRenderableTexture("def_worldNormals", m_gbufferTextures[1].get(), cmdList);
			shader->setRenderableTexture("def_albedo", m_gbufferTextures[2].get(), cmdList);
			shader->setRenderableTexture("def_mrao", m_gbufferTextures[3].get(), cmdList);
		};
		m_shadingPassMaterial.setBindFunc(materialFunc);

		mesh->draw(*this, &m_shadingPassMaterial, shader, environment, cmdList.Get());

	}


	// Copy a gbuffer to the backbuffer for testing purposes
	//auto* renderTarget = m_context->getCurrentRenderTargetResource();
	//DX12Utils::SetResourceTransitionBarrier(cmdList.Get(), renderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);
	//m_gbufferTextures[0]->transitionStateTo(cmdList.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE);
	//cmdList->CopyResource(renderTarget, m_gbufferTextures[0]->getResource());
	//
	//// Transition back buffer to present
	//DX12Utils::SetResourceTransitionBarrier(cmdList.Get(), renderTarget, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);
	//m_context->prepareToRender(cmdList.Get());

	{
		SAIL_PROFILE_API_SPECIFIC_SCOPE("Execution");

		// Lastly - transition back buffer to present
		m_context->prepareToPresent(cmdList.Get());
		// Execute command list
		cmdList->Close();
		m_context->getDirectQueue()->executeCommandLists({ cmdList.Get() });
	}

}
