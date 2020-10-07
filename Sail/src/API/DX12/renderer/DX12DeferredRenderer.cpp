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

DX12DeferredRenderer::GBufferTextures DX12DeferredRenderer::sGBufferTextures;

DX12DeferredRenderer::DX12DeferredRenderer() {
	EventSystem::getInstance()->subscribeToEvent(Event::WINDOW_RESIZE, this);

	auto* app = Application::getInstance();
	m_context = app->getAPI<DX12API>();
	m_context->initCommand(m_command);
	m_command.list->SetName(L"Deferred Renderer main command list");

	auto width = app->getWindow()->getWindowWidth();
	auto height = app->getWindow()->getWindowHeight();

	m_clearColor = glm::vec4(0.2f);
	sGBufferTextures.positions = std::unique_ptr<DX12RenderableTexture>(SAIL_NEW DX12RenderableTexture(width, height, RenderableTexture::USAGE_SAMPLING_ACCESS, "GBuffer positions", ResourceFormat::R16G16B16A16_FLOAT, m_clearColor));
	sGBufferTextures.normals = std::unique_ptr<DX12RenderableTexture>(SAIL_NEW DX12RenderableTexture(width, height, RenderableTexture::USAGE_SAMPLING_ACCESS, "GBuffer normals", ResourceFormat::R16G16B16A16_FLOAT, m_clearColor));
	sGBufferTextures.albedo = std::unique_ptr<DX12RenderableTexture>(SAIL_NEW DX12RenderableTexture(width, height, RenderableTexture::USAGE_SAMPLING_ACCESS, "GBuffer albedo", ResourceFormat::R8G8B8A8, m_clearColor));
	sGBufferTextures.mrao = std::unique_ptr<DX12RenderableTexture>(SAIL_NEW DX12RenderableTexture(width, height, RenderableTexture::USAGE_SAMPLING_ACCESS, "GBuffer mrao", ResourceFormat::R8G8B8A8, m_clearColor));
	sGBufferTextures.depth = std::unique_ptr<DX12RenderableTexture>(SAIL_NEW DX12RenderableTexture(width, height, RenderableTexture::USAGE_SAMPLING_ACCESS, "GBuffer depth", ResourceFormat::DEPTH));

	m_screenQuadModel = ModelFactory::ScreenQuadModel::Create();

	// SSAO
	if (app->getSettings().getBool(Settings::Graphics_SSAO)) {
		m_ssao = std::make_unique<SSAO>();
	}
}

DX12DeferredRenderer::~DX12DeferredRenderer() {
	m_context->waitForGPU();
	EventSystem::getInstance()->unsubscribeFromEvent(Event::WINDOW_RESIZE, this);
	sGBufferTextures.positions.reset();
	sGBufferTextures.normals.reset();
	sGBufferTextures.albedo.reset();
	sGBufferTextures.mrao.reset();
	sGBufferTextures.depth.reset();
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

	// Bind gbuffer RTV and DSV
	{
		D3D12_CPU_DESCRIPTOR_HANDLE rtvHandles[NUM_GBUFFERS];
		rtvHandles[0] = sGBufferTextures.positions->getRtvCDH();
		rtvHandles[1] = sGBufferTextures.normals->getRtvCDH();
		rtvHandles[2] = sGBufferTextures.albedo->getRtvCDH();
		rtvHandles[3] = sGBufferTextures.mrao->getRtvCDH();

		cmdList->OMSetRenderTargets(NUM_GBUFFERS, rtvHandles, false, &getGeometryPassDsv());
	}
	// Transition gbuffers to render target in a batch
	{
		std::vector<DX12ATexture*> textures = {
			sGBufferTextures.positions.get(),
			sGBufferTextures.normals.get(),
			sGBufferTextures.albedo.get(),
			sGBufferTextures.mrao.get(),
		};
		
		std::vector<D3D12_RESOURCE_STATES> statesAfter(textures.size(), D3D12_RESOURCE_STATE_RENDER_TARGET);
		DX12Utils::SetResourceTransitionBarriers(cmdList.Get(), textures, statesAfter);
	}
	// Clear gbuffers
	{
		sGBufferTextures.positions->clear(m_clearColor, cmdList.Get());
		sGBufferTextures.normals->clear(m_clearColor, cmdList.Get());
		sGBufferTextures.albedo->clear(m_clearColor, cmdList.Get());
		sGBufferTextures.mrao->clear(m_clearColor, cmdList.Get());
		sGBufferTextures.depth->clear({}, cmdList.Get());
	}
	cmdList->RSSetViewports(1, m_context->getViewport());
	cmdList->RSSetScissorRects(1, m_context->getScissorRect());

	cmdList->SetGraphicsRootSignature(m_context->getGlobalRootSignature());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return cmdList.Get();
}

void DX12DeferredRenderer::runGeometryPass(ID3D12GraphicsCommandList4* cmdList) {
	SAIL_PROFILE_API_SPECIFIC_FUNCTION("Geometry pass");
	auto& resman = Application::getInstance()->getResourceManager();

	// Iterate unique PSO's
	for (auto it : commandQueue) {
		PipelineStateObject* pso = it.first;
		auto& renderCommands = it.second;
		DX12Shader* shader = static_cast<DX12Shader*>(pso->getShader());

		// Set offset in SRV heap for this mesh 
		cmdList->SetGraphicsRootDescriptorTable(m_context->getRootSignEntryFromRegister("t0").rootSigIndex, m_context->getMainGPUDescriptorHeap()->getCurrentGPUDescriptorHandle());

		shader->updateDescriptorsAndMaterialIndices(renderCommands, *environment, pso, cmdList);

		pso->bind(cmdList);

		shader->trySetCBufferVar("sys_mView", &camera->getViewMatrix(), sizeof(glm::mat4), cmdList);
		shader->trySetCBufferVar("sys_mProjection", &camera->getProjMatrix(), sizeof(glm::mat4), cmdList);
		shader->trySetCBufferVar("sys_mVP", &camera->getViewProjection(), sizeof(glm::mat4), cmdList);

		for (RenderCommand& command : renderCommands) {
			shader->trySetConstantVar("sys_materialIndex", &command.materialIndex, sizeof(unsigned int), cmdList);
			shader->trySetConstantVar("sys_mWorld", &glm::transpose(command.transform), sizeof(glm::mat4), cmdList);
			command.mesh->draw(*this, command.material, shader, cmdList);
		}
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
	sGBufferTextures.positions->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	sGBufferTextures.normals->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);

	// Set offset in SRV heap for this mesh 
	cmdList->SetGraphicsRootDescriptorTable(m_context->getRootSignEntryFromRegister("t0").rootSigIndex, m_context->getMainGPUDescriptorHeap()->getCurrentGPUDescriptorHandle());

	auto* shader = &resman.getShaderSet(Shaders::SSAOShader);
	auto* mesh = m_screenQuadModel->getMesh(0);

	// Find a matching pipelineStateObject and bind it
	auto& pso = resman.getPSO(shader, mesh);
	pso.bind(cmdList);

	shader->trySetCBufferVar("sys_mView", &camera->getViewMatrix(), sizeof(glm::mat4), cmdList);
	shader->trySetCBufferVar("sys_mProjection", &camera->getProjMatrix(), sizeof(glm::mat4), cmdList);
	auto& [kernelData, kernelDataSize] = m_ssao->getKernel();
	shader->trySetCBufferVar("kernel", kernelData, kernelDataSize, cmdList);
	auto& [noiseData, noiseDataSize] = m_ssao->getNoise();
	shader->trySetCBufferVar("noise", noiseData, noiseDataSize, cmdList);
	glm::vec2 ssaoSize(m_ssao->getRenderTargetWidth(), m_ssao->getRenderTargetHeight());
	shader->trySetCBufferVar("windowSize", &ssaoSize, sizeof(glm::vec2), cmdList);
	
	D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Format = DXGI_FORMAT_B8G8R8A8_UNORM;
	m_context->getDevice()->CreateShaderResourceView(nullptr, &srvDesc, m_context->getMainGPUDescriptorHeap()->getNextCPUDescriptorHandle()); // t0
	m_context->getDevice()->CopyDescriptorsSimple(1, m_context->getMainGPUDescriptorHeap()->getNextCPUDescriptorHandle(), sGBufferTextures.positions->getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV); // t1
	m_context->getDevice()->CopyDescriptorsSimple(1, m_context->getMainGPUDescriptorHeap()->getNextCPUDescriptorHandle(), sGBufferTextures.normals->getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV); // t2


	mesh->draw(*this, &m_shadingPassMaterial, shader, cmdList);

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
		blurHorizontalShader.setConstantVar("textureSizeDifference", &textureSizeDiff, sizeof(float), cmdList);

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
		blurVerticalShader.setConstantVar("textureSizeDifference", &textureSizeDiff, sizeof(float), cmdList);

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
	m_context->clearBackBuffer(cmdList); // Not really necessary since the shading pass writes every pixel on the screen
	// Bind the heap
	m_context->getMainGPUDescriptorHeap()->bind(cmdList);

	// Transition gbuffers to pixel shader resources
	{
		std::vector<DX12ATexture*> textures = {
			sGBufferTextures.positions.get(),
			sGBufferTextures.normals.get(),
			sGBufferTextures.albedo.get(),
			sGBufferTextures.mrao.get()
		};

		std::vector<D3D12_RESOURCE_STATES> statesAfter(textures.size(), D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		DX12Utils::SetResourceTransitionBarriers(cmdList, textures, statesAfter);

		if (useSSAO)
			m_ssaoShadingTexture->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	auto* shader = &Application::getInstance()->getResourceManager().getShaderSet(Shaders::DeferredShadingPassShader);
	auto* mesh = m_screenQuadModel->getMesh(0);

	// Find a matching pipelineStateObject and bind it
	auto& pso = resman.getPSO(shader, mesh);
	pso.bind(cmdList);

	shader->trySetCBufferVar("sys_mViewInv", &glm::inverse(camera->getViewMatrix()), sizeof(glm::mat4), cmdList);
	shader->trySetCBufferVar("sys_cameraPos", &camera->getPosition(), sizeof(glm::vec3), cmdList);
	int useSSAOInt = (int)useSSAO;
	shader->trySetCBufferVar("useSSAO", &useSSAOInt, sizeof(int), cmdList);
	int useShadowTextureInt = (int)useDXRHardShadows;
	shader->trySetCBufferVar("useShadowTexture", &useShadowTextureInt, sizeof(int), cmdList);

	if (lightSetup) {
		auto& [dlData, dlDataByteSize] = lightSetup->getDirLightData();
		auto& [plData, plDataByteSize] = lightSetup->getPointLightsData();
		unsigned int numPLs = lightSetup->getNumPLs();
		shader->trySetCBufferVar("dirLight", dlData, dlDataByteSize, cmdList);
		shader->trySetCBufferVar("pointLights", plData, plDataByteSize, cmdList);
		shader->trySetCBufferVar("sys_numPLights", &numPLs, sizeof(unsigned int), cmdList);
	}

	// Bind shader resources
	{
		auto heap = m_context->getMainGPUDescriptorHeap();
		auto dev = m_context->getDevice();

		// Set offset in SRV heap for the 2D texture array
		cmdList->SetGraphicsRootDescriptorTable(m_context->getRootSignEntryFromRegister("t0, space1").rootSigIndex, heap->getCurrentGPUDescriptorHandle());

		// This order needs to match the indexing used in the shader
		auto* brdfLutTexture = &Application::getInstance()->getResourceManager().getTexture("pbr/brdfLUT.tga");
		dev->CopyDescriptorsSimple(1, heap->getNextCPUDescriptorHandle(), static_cast<DX12Texture*>(brdfLutTexture)->getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		dev->CopyDescriptorsSimple(1, heap->getNextCPUDescriptorHandle(), sGBufferTextures.positions->getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		dev->CopyDescriptorsSimple(1, heap->getNextCPUDescriptorHandle(), sGBufferTextures.normals->getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		dev->CopyDescriptorsSimple(1, heap->getNextCPUDescriptorHandle(), sGBufferTextures.albedo->getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		dev->CopyDescriptorsSimple(1, heap->getNextCPUDescriptorHandle(), sGBufferTextures.mrao->getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);

		if (m_ssao) {
			dev->CopyDescriptorsSimple(1, heap->getNextCPUDescriptorHandle(), m_ssaoShadingTexture->getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		} else {
			// Skip this slot in the heap
			heap->getAndStepIndex(1);
		}

		if (useDXRHardShadows) {
			dev->CopyDescriptorsSimple(1, heap->getNextCPUDescriptorHandle(), DX12RaytracingRenderer::GetOutputTexture()->get()->getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		} else {
			// Skip this slot in the heap
			heap->getAndStepIndex(1);
		}

		// Set offset in SRV heap for the texture cube array
		cmdList->SetGraphicsRootDescriptorTable(m_context->getRootSignEntryFromRegister("t0, space2").rootSigIndex, heap->getCurrentGPUDescriptorHandle());

		auto radiance = static_cast<DX12Texture*>(environment->getRadianceTexture());
		radiance->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		dev->CopyDescriptorsSimple(1, heap->getNextCPUDescriptorHandle(), radiance->getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
		auto irradiance = static_cast<DX12Texture*>(environment->getIrradianceTexture());
		irradiance->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
		dev->CopyDescriptorsSimple(1, heap->getNextCPUDescriptorHandle(), irradiance->getSrvCDH(), D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	//// Inline raytracing test - bind AS
	//D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	//srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
	//srvDesc.RaytracingAccelerationStructure.Location = DXRBase::GetTLASAddress();
	//srvDesc.ViewDimension = D3D12_SRV_DIMENSION_RAYTRACING_ACCELERATION_STRUCTURE;
	//m_context->getDevice()->CreateShaderResourceView(nullptr, &srvDesc, m_context->getMainGPUDescriptorHeap()->getNextCPUDescriptorHandle());

	mesh->draw(*this, nullptr, shader, cmdList);
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
		sGBufferTextures.positions->resize(event.getWidth(), event.getHeight());
		sGBufferTextures.normals->resize(event.getWidth(), event.getHeight());
		sGBufferTextures.albedo->resize(event.getWidth(), event.getHeight());
		sGBufferTextures.mrao->resize(event.getWidth(), event.getHeight());
		sGBufferTextures.depth->resize(event.getWidth(), event.getHeight());

		if (m_ssao)
			m_ssao->resize(event.getWidth(), event.getHeight());
		return true;
	};
	EventHandler::HandleType<WindowResizeEvent>(event, resizeEvent);
	return true;
}

const DX12DeferredRenderer::GBufferTextures& DX12DeferredRenderer::GetGBuffers() {
	return sGBufferTextures;
}

D3D12_CPU_DESCRIPTOR_HANDLE DX12DeferredRenderer::getGeometryPassDsv() {
	return sGBufferTextures.depth->getDsvCDH();
}
