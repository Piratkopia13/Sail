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
#include "../shader/DX12ComputeShaderDispatcher.h"
#include "DX12RaytracingRenderer.h"

std::unique_ptr<DX12RenderableTexture> DX12DeferredRenderer::sGBufferTextures[NUM_GBUFFERS];

DX12DeferredRenderer::DX12DeferredRenderer() {
	EventSystem::getInstance()->subscribeToEvent(Event::WINDOW_RESIZE, this);

	auto* app = Application::getInstance();
	m_context = app->getAPI<DX12API>();
	m_context->initCommand(m_command);
	m_command.list->SetName(L"Deferred Renderer main command list");

	auto windowWidth = app->getWindow()->getWindowWidth();
	auto windowHeight = app->getWindow()->getWindowHeight();

	for (int i = 0; i < NUM_GBUFFERS; i++) {
		glm::vec4 clearColor(0.f);
		clearColor.z = (i == 0) ? FLT_MAX : 0.f; // Position texture z needs this for ssao to work with skybox in background
		sGBufferTextures[i] = std::unique_ptr<DX12RenderableTexture>(static_cast<DX12RenderableTexture*>(
			RenderableTexture::Create(windowWidth, windowHeight, "GBuffer renderer output " + std::to_string(i), (i < 2) ? ResourceFormat::R16G16B16A16_FLOAT : ResourceFormat::R8G8B8A8, (i == 0), false, clearColor)));
	}

	m_screenQuadModel = ModelFactory::ScreenQuadModel::Create();

	// SSAO
	if (app->getSettings().getBool(Settings::Graphics_SSAO)) {
		m_ssao = std::make_unique<SSAO>();
	}
}

DX12DeferredRenderer::~DX12DeferredRenderer() {
	EventSystem::getInstance()->unsubscribeFromEvent(Event::WINDOW_RESIZE, this);
	for (unsigned int i = 0; i < NUM_GBUFFERS; i++) {
		sGBufferTextures[i].reset();
	}
}

void* DX12DeferredRenderer::present(Renderer::PresentFlag flags, void* skippedPrepCmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION();

	bool doSSAO = Application::getInstance()->getSettings().getBool(Settings::Graphics_SSAO);
	if (!m_ssao && doSSAO) {
		// Handle enabling of ssao in runtime
		m_ssao = std::make_unique<SSAO>();
	} else if (m_ssao && !doSSAO) {
		// Handle disabling of ssao in runtime
		m_context->waitForGPU();
		m_ssao.reset();
	}

	ID3D12GraphicsCommandList4* cmdList = nullptr;
	if (flags & Renderer::PresentFlag::SkipPreparation) {
		if (skippedPrepCmdList)
			cmdList = static_cast<ID3D12GraphicsCommandList4*>(skippedPrepCmdList);
		else
			Logger::Error("DX12DeferredRenderer present was called with skipPreparation flag but no cmdList was passed");
	}

	if (!(flags & Renderer::PresentFlag::SkipPreparation))
		cmdList = runFramePreparation();
	if (!(flags & Renderer::PresentFlag::SkipRendering)) {
		runGeometryPass(cmdList);
		if (doSSAO)
			runSSAO(cmdList);
	}
	if (!(flags & Renderer::PresentFlag::SkipDeferredShading))
		runShadingPass(cmdList);
	if (!(flags & Renderer::PresentFlag::SkipExecution))
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
		rtvHandles[i] = sGBufferTextures[i]->getRtvCDH();
	}
	cmdList->OMSetRenderTargets(NUM_GBUFFERS, rtvHandles, false, &getGeometryPassDsv());
	// Transition gbuffers to render target
	for (int i = 0; i < NUM_GBUFFERS; i++) {
		// TODO: transition in batch
		sGBufferTextures[i]->transitionStateTo(cmdList.Get(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		float z = (i == 0) ? FLT_MAX : 0.f;
		sGBufferTextures[i]->clear({ 0.0f, 0.0f, z, 0.0f }, cmdList.Get());
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

void DX12DeferredRenderer::runSSAO(ID3D12GraphicsCommandList4* cmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION("SSAO");
	auto& resman = Application::getInstance()->getResourceManager();

	auto* ssaoRenderTarget = static_cast<DX12RenderableTexture*>(m_ssao->getRenderTargetTexture());
	ssaoRenderTarget->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
	ssaoRenderTarget->clear({ 0.0f, 0.0f, 0.0f, 0.0f }, cmdList);
	cmdList->OMSetRenderTargets(1, &ssaoRenderTarget->getRtvCDH(), true, nullptr);

	D3D12_VIEWPORT viewport = { 0 };
	viewport.Width = m_ssao->getRenderTargetWidth();
	viewport.Height = m_ssao->getRenderTargetHeight();
	viewport.MaxDepth = 1.0f;
	D3D12_RECT scissorRect = { 0 };
	scissorRect.right = (long)m_ssao->getRenderTargetWidth();
	scissorRect.bottom = (long)m_ssao->getRenderTargetHeight();

	cmdList->RSSetViewports(1, &viewport);
	cmdList->RSSetScissorRects(1, &scissorRect);


	// Transition gbuffers to pixel shader resources
	sGBufferTextures[0]->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	sGBufferTextures[1]->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	auto* shader = &Application::getInstance()->getResourceManager().getShaderSet(Shaders::SSAOShader);
	auto* mesh = m_screenQuadModel->getMesh(0);

	// Find a matching pipelineStateObject and bind it
	auto& pso = resman.getPSO(shader, mesh);
	pso.bind(cmdList);

	shader->trySetCBufferVar("sys_mView", &camera->getViewMatrix(), sizeof(glm::mat4));
	shader->trySetCBufferVar("sys_mProjection", &camera->getProjMatrix(), sizeof(glm::mat4));
	auto& [kernelData, kernelDataSize] = m_ssao->getKernel();
	shader->trySetCBufferVar("kernel", kernelData, kernelDataSize);
	auto& [noiseData, noiseDataSize] = m_ssao->getNoise();
	shader->trySetCBufferVar("noise", noiseData, noiseDataSize);
	glm::vec2 ssaoSize(m_ssao->getRenderTargetWidth(), m_ssao->getRenderTargetHeight());
	shader->trySetCBufferVar("windowSize", &ssaoSize, sizeof(glm::vec2));

	auto materialFunc = [&](Shader* shader, Environment* environment, void* cmdList) {
		// Bind GBuffer textures
		shader->setRenderableTexture("def_positions", sGBufferTextures[0].get(), cmdList);
		shader->setRenderableTexture("def_worldNormals", sGBufferTextures[1].get(), cmdList);
	};
	m_shadingPassMaterial.setBindFunc(materialFunc);

	mesh->draw(*this, &m_shadingPassMaterial, shader, environment, cmdList);

	// Blur ssao output
	auto& blurHorizontalShader = Application::getInstance()->getResourceManager().getShaderSet(Shaders::GaussianBlurHorizontalComputeShader);
	auto& blurVerticalShader = Application::getInstance()->getResourceManager().getShaderSet(Shaders::GaussianBlurVerticalComputeShader);

	auto& settingsHorizontal = blurHorizontalShader.getSettings().computeShaderSettings;
	auto& settingsVertical = blurVerticalShader.getSettings().computeShaderSettings;
	float textureSizeDiff = 1.f;

	const auto& heap = m_context->getMainGPUDescriptorHeap();
	DX12ComputeShaderDispatcher csDispatcher;
	csDispatcher.begin(cmdList);

	// Dispatch horizontal blur pass
	{
		blurHorizontalShader.setCBufferVar("textureSizeDifference", &textureSizeDiff, sizeof(float));

		DescriptorHeap::DescriptorTableInstanceBuilder instance;

		instance.add("t0", [&](auto cpuHandle) { }); // TODO: figure out why this line is needed for u10 to bind (something is probably wrong in addAndBind)
		instance.add("u10", [&](auto cpuHandle) {
			m_context->getDevice()->CopyDescriptorsSimple(1, cpuHandle, ssaoRenderTarget->getUavCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			ssaoRenderTarget->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		});

		// Add the instance to the heap
		// This binds resource views in the right places in the heap according to the root signature
		heap->addAndBind(instance, cmdList, true);

		unsigned int x = (unsigned int)glm::ceil(m_ssao->getRenderTargetWidth() * settingsHorizontal.threadGroupXScale);
		unsigned int y = (unsigned int)glm::ceil(m_ssao->getRenderTargetHeight() * settingsHorizontal.threadGroupYScale);
		unsigned int z = 1;
		csDispatcher.dispatch(blurHorizontalShader, { x, y, z }, cmdList);
	}

	// Dispatch vertical blur pass
	{
		blurVerticalShader.setCBufferVar("textureSizeDifference", &textureSizeDiff, sizeof(float));

		DescriptorHeap::DescriptorTableInstanceBuilder instance;

		m_ssaoShadingTexture = static_cast<DX12RenderableTexture*>(blurVerticalShader.getRenderableTexture("output"));
		m_ssaoShadingTexture->resize(m_ssao->getRenderTargetWidth(), m_ssao->getRenderTargetHeight());
		instance.add("t0", [&](auto cpuHandle) {
			m_context->getDevice()->CopyDescriptorsSimple(1, cpuHandle, ssaoRenderTarget->getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			ssaoRenderTarget->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);
		});
		instance.add("u10", [&](auto cpuHandle) {
			m_context->getDevice()->CopyDescriptorsSimple(1, cpuHandle, m_ssaoShadingTexture->getUavCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
			m_ssaoShadingTexture->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
		});

		heap->addAndBind(instance, cmdList, true);

		unsigned int x = (unsigned int)glm::ceil(m_ssao->getRenderTargetWidth() * settingsVertical.threadGroupXScale);
		unsigned int y = (unsigned int)glm::ceil(m_ssao->getRenderTargetHeight() * settingsVertical.threadGroupYScale);
		unsigned int z = 1;
		csDispatcher.dispatch(blurVerticalShader, { x, y, z }, cmdList);
	}
	//m_ssaoBlurredTexture = m_ssaoOutputTexture.get();
}

void DX12DeferredRenderer::runShadingPass(ID3D12GraphicsCommandList4* cmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION("Shading pass");
	auto& resman = Application::getInstance()->getResourceManager();
	bool useSSAO = Application::getInstance()->getSettings().getBool(Settings::Graphics_SSAO);
	bool useDXRHardShadows = Application::getInstance()->getSettings().getBool(Settings::Graphics_DXR);

	// Transition back buffer to render target
	m_context->prepareToRender(cmdList);

	// Set back buffer as render target
	m_context->renderToBackBuffer(cmdList);
	//m_context->clearBackBuffer(cmdList);
	// Bind the heap
	m_context->getMainGPUDescriptorHeap()->bind(cmdList);

	// Transition gbuffers to pixel shader resources
	for (int i = 0; i < NUM_GBUFFERS; i++)
		sGBufferTextures[i]->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE); // TODO: transition in batch
	if (useSSAO)
		m_ssaoShadingTexture->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	auto* shader = &Application::getInstance()->getResourceManager().getShaderSet(Shaders::DeferredShadingPassShader);
	auto* mesh = m_screenQuadModel->getMesh(0);

	// Find a matching pipelineStateObject and bind it
	auto& pso = resman.getPSO(shader, mesh);
	pso.bind(cmdList);

	shader->trySetCBufferVar("sys_mViewInv", &glm::inverse(camera->getViewMatrix()), sizeof(glm::mat4));
	shader->trySetCBufferVar("sys_cameraPos", &camera->getPosition(), sizeof(glm::vec3));
	int useSSAOInt = (int)useSSAO;
	shader->trySetCBufferVar("useSSAO", &useSSAOInt, sizeof(int));
	int useShadowTextureInt = (int)useDXRHardShadows;
	shader->trySetCBufferVar("useShadowTexture", &useShadowTextureInt, sizeof(int));

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
		shader->setRenderableTexture("def_positions", sGBufferTextures[0].get(), cmdList);
		shader->setRenderableTexture("def_worldNormals", sGBufferTextures[1].get(), cmdList);
		shader->setRenderableTexture("def_albedo", sGBufferTextures[2].get(), cmdList);
		shader->setRenderableTexture("def_mrao", sGBufferTextures[3].get(), cmdList);
		if (useSSAO)
			shader->setRenderableTexture("tex_ssao", m_ssaoShadingTexture, cmdList);
		else 
			shader->setRenderableTexture("tex_ssao", nullptr, cmdList);
		if (useDXRHardShadows)
			shader->setRenderableTexture("tex_shadows", DX12RaytracingRenderer::GetOutputTexture()->get(), cmdList);
		else 
			shader->setRenderableTexture("tex_shadows", nullptr, cmdList);
		
		//// Inline raytracing test - bind AS
		//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		//srvDesc.RaytracingAccelerationStructure.Location = DXRBase::GetTLASAddress();
		//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
		//m_context->getDevice()->CreateShaderResourceView(nullptr, &srvDesc, m_context->getMainGPUDescriptorHeap()->getNextCPUDescriptorHandle());

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

bool DX12DeferredRenderer::onEvent(Event& event) {
	auto resizeEvent = [&](WindowResizeEvent& event) {
		for (unsigned i = 0; i < NUM_GBUFFERS; i++) {
			sGBufferTextures[i]->resize(event.getWidth(), event.getHeight());
		}

		return true;
	};
	EventHandler::HandleType<WindowResizeEvent>(event, resizeEvent);
	return true;
}

const std::unique_ptr<DX12RenderableTexture>* DX12DeferredRenderer::GetGBuffers() {
	return sGBufferTextures;
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12DeferredRenderer::getGeometryPassDsv() {
	return sGBufferTextures[0]->getDsvCDH();
}
