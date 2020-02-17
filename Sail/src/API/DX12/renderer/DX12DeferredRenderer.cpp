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

	m_screenQuadModel = ModelFactory::ScreenQuadModel::Create();
}

DX12DeferredRenderer::~DX12DeferredRenderer() {

}

void* DX12DeferredRenderer::present(Renderer::RenderFlag flags, void* skippedPrepCmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	ID3D12GraphicsCommandList4* cmdList = nullptr;
	if (flags & Renderer::RenderFlag::SkipPreparation) {
		if (skippedPrepCmdList)
			cmdList = static_cast<ID3D12GraphicsCommandList4*>(skippedPrepCmdList);
		else
			Logger::Error("DX12ForwardRenderer present was called with skipPreparation flag but no cmdList was passed");
	}

	if (!(flags & Renderer::RenderFlag::SkipPreparation))
		cmdList = runFramePreparation();
	if (!(flags & Renderer::RenderFlag::SkipRendering)) {
		runGeometryPass(cmdList);
		runShadingPass(cmdList);
	}
	if (!(flags & Renderer::RenderFlag::SkipExecution))
		runFrameExecution(cmdList);

	return cmdList;
}

void* DX12DeferredRenderer::getDepthBuffer() {
	return (void*)getGeometryPassDsv().ptr;
}

ID3D12GraphicsCommandList4* DX12DeferredRenderer::runFramePreparation() {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION("Frame preparation");
	auto frameIndex = m_context->getFrameIndex();

	auto& allocator = m_command.allocators[frameIndex];
	auto& cmdList = m_command.list;


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
	cmdList->OMSetRenderTargets(NUM_GBUFFERS, rtvHandles, false, &getGeometryPassDsv());
	// Transition gbuffers to render target
	for (int i = 0; i < NUM_GBUFFERS; i++) {
		// TODO: transition in batch
		m_gbufferTextures[i]->transitionStateTo(cmdList.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		m_gbufferTextures[i]->clear({ 0.0f, 0.0f, 0.0f, 0.0f }, cmdList.Get());
	}
	cmdList->RSSetViewports(1, m_context->getViewport());
	cmdList->RSSetScissorRects(1, m_context->getScissorRect());

	cmdList->SetGraphicsRootSignature(m_context->getGlobalRootSignature());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	// Bind mesh-common constant buffers (camera)
	// TODO: bind camera cbuffer here
	//cmdList->SetGraphicsRootConstantBufferView(GlobalRootParam::CBV_CAMERA, asdf);

	return cmdList.Get();
}


void DX12DeferredRenderer::runGeometryPass(ID3D12GraphicsCommandList4* cmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION("Geometry pass");
	auto& resman = Application::getInstance()->getResourceManager();

	// TODO: Sort meshes according to shaderPipeline
	unsigned int totalInstances = commandQueue.size();
	for (RenderCommand& command : commandQueue) {
		DX12Shader* shader = static_cast<DX12Shader*>(command.shader);
		if (!shader) {
			Logger::Warning("Tried to render a model with no shader set");
			continue;
		}
		//uniqueShaderPipelines.insert(shaderPipeline);

		// Make sure that constant buffers have a size that can allow the amount of meshes that will be rendered this frame
		shader->reserve(totalInstances);

		// Find a matching pipelineStateObject and bind it
		auto& pso = resman.getPSO(shader, command.mesh);
		pso.bind(cmdList);

		shader->trySetCBufferVar("sys_mWorld", &glm::transpose(command.transform), sizeof(glm::mat4));
		shader->trySetCBufferVar("sys_mView", &camera->getViewMatrix(), sizeof(glm::mat4));
		shader->trySetCBufferVar("sys_mProjection", &camera->getProjMatrix(), sizeof(glm::mat4));
		shader->trySetCBufferVar("sys_mVP", &camera->getViewProjection(), sizeof(glm::mat4));

		command.mesh->draw(*this, command.material, shader, environment, cmdList);
	}
}

void DX12DeferredRenderer::runShadingPass(ID3D12GraphicsCommandList4* cmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION("Shading pass");
	auto& resman = Application::getInstance()->getResourceManager();

	// Transition back buffer to render target
	m_context->prepareToRender(cmdList);

	// Set back buffer as render target
	m_context->renderToBackBuffer(cmdList);
	//m_context->clearBackBuffer(cmdList);

	// Transition gbuffers to pixel shader resources
	for (int i = 0; i < NUM_GBUFFERS; i++)
		m_gbufferTextures[i]->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); // TODO: transition in batch

	auto* shader = &Application::getInstance()->getResourceManager().getShaderSet(Shaders::DeferredShadingPassShader);
	auto* mesh = m_screenQuadModel->getMesh(0);

	// Find a matching pipelineStateObject and bind it
	auto& pso = resman.getPSO(shader, mesh);
	pso.bind(cmdList);

	shader->trySetCBufferVar("sys_mViewInv", &glm::inverse(camera->getViewMatrix()), sizeof(glm::mat4));
	shader->trySetCBufferVar("sys_cameraPos", &camera->getPosition(), sizeof(glm::vec3));

	if (lightSetup) {
		auto& [dlData, dlDataByteSize] = lightSetup->getDirLightData();
		auto& [plData, plDataByteSize] = lightSetup->getPointLightsData();
		unsigned int numPLs = lightSetup->getNumPLs();
		shader->trySetCBufferVar("dirLight", dlData, dlDataByteSize);
		shader->trySetCBufferVar("pointLights", plData, plDataByteSize);
		shader->trySetCBufferVar("sys_numPLights", &numPLs, sizeof(unsigned int));
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

	mesh->draw(*this, &m_shadingPassMaterial, shader, environment, cmdList);
}

void DX12DeferredRenderer::runFrameExecution(ID3D12GraphicsCommandList4* cmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION("Execution");

	// Lastly - transition back buffer to present
	m_context->prepareToPresent(cmdList);
	// Execute command list
	cmdList->Close();
	m_context->getDirectQueue()->executeCommandLists({ cmdList });
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12DeferredRenderer::getGeometryPassDsv() {
	return m_gbufferTextures[0]->getDsvCDH();
}