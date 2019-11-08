#include "pch.h"
#include "DX12RaytracingRenderer.h"
#include "Sail/Application.h"
#include "../DX12Utils.h"
#include "Sail/graphics/postprocessing/PostProcessPipeline.h"
#include "API/DX12/DX12VertexBuffer.h"
#include "Sail/KeyBinds.h"
#include "Sail/entities/components/MapComponent.h"
#include "Sail/graphics/geometry/factory/ScreenQuadModel.h"
#include "Sail/graphics/shader/dxr/ShadePassShader.h"
#include "API/DX12/shader/DX12ShaderPipeline.h"
#include "Sail/graphics/light/LightSetup.h"

// Current goal is to make this render a fully raytraced image of all geometry (without materials) within a scene

DX12RaytracingRenderer::DX12RaytracingRenderer(DX12RenderableTexture** inputs)
	: m_dxr("Basic", inputs)
	, m_gbufferTextures(inputs)
{
	lightSetup = nullptr;
	Application* app = Application::getInstance();
	m_context = app->getAPI<DX12API>();
	m_context->initCommand(m_commandDirect, D3D12_COMMAND_LIST_TYPE_DIRECT, L"Raytracing Renderer DIRECT command list or allocator");
	m_context->initCommand(m_commandCompute, D3D12_COMMAND_LIST_TYPE_COMPUTE, L"Raytracing Renderer main COMPUTE command list or allocator");

	auto windowWidth = app->getWindow()->getWindowWidth();
	auto windowHeight = app->getWindow()->getWindowHeight();

	// Initialize raytracing output textures
	// m_outputTextures contains all reflection bounce information
	// gbuffer textures contains first "bounce" information
	m_outputTextures.albedo = std::unique_ptr<DX12RenderableTexture>(static_cast<DX12RenderableTexture*>(RenderableTexture::Create(windowWidth, windowHeight)));
	m_outputTextures.metalnessRoughnessAO = std::unique_ptr<DX12RenderableTexture>(static_cast<DX12RenderableTexture*>(RenderableTexture::Create(windowWidth, windowHeight)));
	m_outputTextures.normal = std::unique_ptr<DX12RenderableTexture>(static_cast<DX12RenderableTexture*>(RenderableTexture::Create(windowWidth, windowHeight)));
	m_outputTextures.shadows = std::unique_ptr<DX12RenderableTexture>(static_cast<DX12RenderableTexture*>(RenderableTexture::Create(windowWidth, windowHeight, "CurrentFrameShadowTexture", Texture::R8G8)));
	m_outputTextures.depthPositions = std::unique_ptr<DX12RenderableTexture>(static_cast<DX12RenderableTexture*>(RenderableTexture::Create(windowWidth, windowHeight, "DepthPositionTexture", Texture::R16G16B16A16_FLOAT)));
	// init shaded output texture - this is written to in a rasterisation pass
	m_shadedOuput = std::unique_ptr<DX12RenderableTexture>(static_cast<DX12RenderableTexture*>(RenderableTexture::Create(windowWidth, windowHeight)));
	// Init raytracing input texture
	m_shadowsLastFrame = std::unique_ptr<DX12RenderableTexture>(static_cast<DX12RenderableTexture*>(RenderableTexture::Create(windowWidth, windowHeight, "LastFrameShadowTexture", Texture::R8G8)));

	m_brdfTexture = &app->getResourceManager().getTexture("pbr/brdfLUT.tga");

	m_shadeShader = &app->getResourceManager().getShaderSet<ShadePassShader>();
	m_fullscreenModel = ModelFactory::ScreenQuadModel::Create(m_shadeShader);

	m_currNumDecals = 0;
	memset(m_decals, 0, sizeof DXRShaderCommon::DecalData * MAX_DECALS);
}

DX12RaytracingRenderer::~DX12RaytracingRenderer() {

}

void DX12RaytracingRenderer::present(PostProcessPipeline* postProcessPipeline, RenderableTexture* output) {
	auto frameIndex = m_context->getSwapIndex();

	// There is one allocator per swap buffer
	auto& allocatorCompute = m_commandCompute.allocators[m_context->getFrameIndex()];
	auto& allocatorDirect = m_commandDirect.allocators[m_context->getFrameIndex()];
	auto& cmdListCompute = m_commandCompute.list;
	auto& cmdListDirect = m_commandDirect.list;

	// Reset allocators and lists for this frame
	allocatorCompute->Reset();
	allocatorDirect->Reset();
	cmdListCompute->Reset(allocatorCompute.Get(), nullptr);
	cmdListDirect->Reset(allocatorDirect.Get(), nullptr);

	// Wait for the G-Buffer pass to finish execution on the direct queue
	auto fenceVal = m_context->getDirectQueue()->signal();
	m_context->getComputeQueue()->wait(fenceVal);

	// Clear output texture
	//m_outputTexture.get()->clear({ 0.01f, 0.01f, 0.01f, 1.0f }, cmdListDirect.Get());

	std::sort(m_metaballs.begin(), m_metaballs.end(),
		[](const DXRBase::Metaball& a, const DXRBase::Metaball& b) -> const bool
		{
			return a.distToCamera < b.distToCamera;
		});

	if (m_metaballs.size() > 0) {
		commandQueue.emplace_back();
		RenderCommand& cmd = commandQueue.back();
		cmd.type = RENDER_COMMAND_TYPE_NON_MODEL_METABALL;
		cmd.flags = Renderer::MESH_DYNAMIC;
		cmd.nonModel.material = nullptr;
		cmd.transform = glm::identity<glm::mat4>();
		cmd.transform = glm::transpose(cmd.transform);
		cmd.hasUpdatedSinceLastRender.resize(m_context->getNumGPUBuffers(), true);

		//Calculate the needed size of m_next_metaball_aabb.
		glm::vec3& pos = m_metaballs.front().pos;
		m_nextMetaballAabb.MaxX = pos.x + METABALL_RADIUS;
		m_nextMetaballAabb.MaxY = pos.y + METABALL_RADIUS;
		m_nextMetaballAabb.MaxZ = pos.z + METABALL_RADIUS;
		m_nextMetaballAabb.MinX = pos.x - METABALL_RADIUS;
		m_nextMetaballAabb.MinY = pos.y - METABALL_RADIUS;
		m_nextMetaballAabb.MinZ = pos.z - METABALL_RADIUS;
		for (size_t i = 1; i < m_metaballs.size() && i < MAX_NUM_METABALLS; i++) {
			glm::vec3& pos = m_metaballs[i].pos;

			if (m_nextMetaballAabb.MaxX < pos.x + METABALL_RADIUS) {
				m_nextMetaballAabb.MaxX = pos.x + METABALL_RADIUS;
			}
			if (m_nextMetaballAabb.MaxY < pos.y + METABALL_RADIUS) {
				m_nextMetaballAabb.MaxY = pos.y + METABALL_RADIUS;
			}
			if (m_nextMetaballAabb.MaxZ < pos.z + METABALL_RADIUS) {
				m_nextMetaballAabb.MaxZ = pos.z + METABALL_RADIUS;
			}

			if (m_nextMetaballAabb.MinX > pos.x - METABALL_RADIUS) {
				m_nextMetaballAabb.MinX = pos.x - METABALL_RADIUS;
			}
			if (m_nextMetaballAabb.MinY > pos.y - METABALL_RADIUS) {
				m_nextMetaballAabb.MinY = pos.y - METABALL_RADIUS;
			}
			if (m_nextMetaballAabb.MinZ > pos.z - METABALL_RADIUS) {
				m_nextMetaballAabb.MinZ = pos.z - METABALL_RADIUS;
			}
		}
	}

	if (Input::WasKeyJustPressed(KeyBinds::RELOAD_DXR_SHADER)) {
		m_dxr.reloadShaders();
	}

	// Copy updated flag from vertex buffers to renderCommand
	// This marks all commands that should have their bottom layer updated in-place
	for (auto& renderCommand : commandQueue) {
		if (renderCommand.type == RENDER_COMMAND_TYPE_MODEL) {
			auto& vb = static_cast<DX12VertexBuffer&>(renderCommand.model.mesh->getVertexBuffer());
			if (vb.hasBeenUpdated()) {
				renderCommand.hasUpdatedSinceLastRender[frameIndex] = true;
				vb.resetHasBeenUpdated();
			}
		}
	}

	if (camera && lightSetup) {
		static auto mapSize = glm::vec3(MapComponent::xsize, 1.0f, MapComponent::ysize) * static_cast<float>(MapComponent::tileSize);
		static auto mapStart = -glm::vec3(MapComponent::tileSize / 2.0f);
		m_dxr.updateSceneData(*camera, *lightSetup, m_metaballs, m_nextMetaballAabb, mapSize, mapStart);

	}

	m_dxr.updateDecalData(m_decals, m_currNumDecals > MAX_DECALS - 1 ? MAX_DECALS : m_currNumDecals);
	m_dxr.updateWaterData();
	m_dxr.updateAccelerationStructures(commandQueue, cmdListCompute.Get());
	m_dxr.dispatch(m_outputTextures, m_shadowsLastFrame.get(), cmdListCompute.Get());
	// AS has now been updated this frame, reset flag
	for (auto& renderCommand : commandQueue) {
		renderCommand.hasUpdatedSinceLastRender[frameIndex] = false;
	}

	auto filteredShadows = runDenoising(cmdListCompute.Get());

	auto shadedOutput = runShading(cmdListDirect.Get(), filteredShadows);

	// TODO: move this to a graphics queue when current cmdList is executed on the compute queue

	RenderableTexture* renderOutput = shadedOutput;
	if (postProcessPipeline) {
		// Run post processing
		RenderableTexture* ppOutput = postProcessPipeline->run(shadedOutput, cmdListCompute.Get());
		if (ppOutput) {
			renderOutput = ppOutput;
		}
	}
	//DX12RenderableTexture* dxRenderOutput = static_cast<DX12RenderableTexture*>(renderOutput);
	DX12RenderableTexture* dxRenderOutput = static_cast<DX12RenderableTexture*>(shadedOutput);
	

	// Execute compute command list
	cmdListCompute->Close();
	m_context->executeCommandLists({ cmdListCompute.Get() }, D3D12_COMMAND_LIST_TYPE_COMPUTE);
	// Place a signal to syncronize copying the raytracing output to the backbuffer when it is available
	fenceVal = m_context->getComputeQueue()->signal();

	// Copy post processing output to back buffer
	dxRenderOutput->transitionStateTo(cmdListDirect.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE);
	auto* renderTarget = m_context->getCurrentRenderTargetResource();
	DX12Utils::SetResourceTransitionBarrier(cmdListDirect.Get(), renderTarget, D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_COPY_DEST);
	cmdListDirect->CopyResource(renderTarget, dxRenderOutput->getResource());
	// Lastly - transition back buffer to present
	DX12Utils::SetResourceTransitionBarrier(cmdListDirect.Get(), renderTarget, D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_PRESENT);

	// Copy output shadow texture to last shadow texture
	m_outputTextures.shadows->transitionStateTo(cmdListDirect.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE);
	m_shadowsLastFrame->transitionStateTo(cmdListDirect.Get(), D3D12_RESOURCE_STATE_COPY_DEST);
	cmdListDirect->CopyResource(m_shadowsLastFrame->getResource(), m_outputTextures.shadows->getResource());
	m_outputTextures.shadows->transitionStateTo(cmdListDirect.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS);
	m_shadowsLastFrame->transitionStateTo(cmdListDirect.Get(), D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	// Wait for compute to finish before executing the direct queue
	m_context->getDirectQueue()->wait(fenceVal);
	// Execute direct command list
	cmdListDirect->Close();
	m_context->executeCommandLists({ cmdListDirect.Get() }, D3D12_COMMAND_LIST_TYPE_DIRECT);
}

DX12RenderableTexture* DX12RaytracingRenderer::runDenoising(ID3D12GraphicsCommandList4* cmdList) {
	Application* app = Application::getInstance();
	const auto windowWidth = app->getWindow()->getWindowWidth();
	const auto windowHeight = app->getWindow()->getWindowHeight();

	cmdList->SetComputeRootSignature(m_context->getGlobalRootSignature());
	m_context->getComputeGPUDescriptorHeap()->bind(cmdList);
	
	m_outputTextures.albedo->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE);

	auto& blurShaderHorizontal = app->getResourceManager().getShaderSet<BilateralBlurHorizontal>();
	auto& blurShaderVertical = app->getResourceManager().getShaderSet<BilateralBlurVertical>();

	// Horizontal pass
	PostProcessPipeline::PostProcessInput input;
	auto* settings = blurShaderHorizontal.getComputeSettings();
	input.inputRenderableTexture = m_outputTextures.shadows.get();
	input.outputWidth = static_cast<unsigned int>(windowWidth);
	input.outputHeight = static_cast<unsigned int>(windowHeight);
	input.threadGroupCountX = static_cast<unsigned int>(glm::ceil(settings->threadGroupXScale * input.outputWidth));
	input.threadGroupCountY = static_cast<unsigned int>(glm::ceil(settings->threadGroupYScale * input.outputHeight));
	auto output = static_cast<PostProcessPipeline::PostProcessOutput&>(m_computeShaderDispatcher.dispatch(blurShaderHorizontal, input, 0, cmdList));

	// Vertical pass
	settings = blurShaderVertical.getComputeSettings();
	input.inputRenderableTexture = output.outputTexture;
	input.threadGroupCountX = static_cast<unsigned int>(glm::ceil(settings->threadGroupXScale * input.outputWidth));
	input.threadGroupCountY = static_cast<unsigned int>(glm::ceil(settings->threadGroupYScale * input.outputHeight));
	output = static_cast<PostProcessPipeline::PostProcessOutput&>(m_computeShaderDispatcher.dispatch(blurShaderVertical, input, 1, cmdList));

	return static_cast<DX12RenderableTexture*>(output.outputTexture);
}

DX12RenderableTexture* DX12RaytracingRenderer::runShading(ID3D12GraphicsCommandList4* cmdList, DX12RenderableTexture* shadows) {
	auto shaderPipeline = static_cast<DX12ShaderPipeline*>(m_shadeShader->getPipeline());

	// Make sure model has been initialized
	static_cast<DX12VertexBuffer&>(m_fullscreenModel->getMesh(0)->getVertexBuffer()).init(cmdList);

	m_shadedOuput->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_RENDER_TARGET);
	cmdList->OMSetRenderTargets(1, &m_shadedOuput->getRtvCDH(), false, nullptr);
	cmdList->RSSetViewports(1, m_context->getViewport());
	cmdList->RSSetScissorRects(1, m_context->getScissorRect());

	cmdList->SetGraphicsRootSignature(m_context->getGlobalRootSignature());
	cmdList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	// Bind the descriptor heap that will contain all SRVs, DSVs and RTVs for this frame
	m_context->getMainGPUDescriptorHeap()->bind(cmdList);

	shaderPipeline->bind_new(cmdList, 0);

	// The following order must match the t-register order in shader
	int numCustomSRVs = 0;
	shaderPipeline->setTexture2D("albedoBounceOne", m_gbufferTextures[1], cmdList); numCustomSRVs++;
	shaderPipeline->setTexture2D("albedoBounceTwo", m_outputTextures.albedo.get(), cmdList); numCustomSRVs++;

	shaderPipeline->setTexture2D("normalsBounceOne", m_gbufferTextures[0], cmdList); numCustomSRVs++;
	shaderPipeline->setTexture2D("normalsBounceTwo", m_outputTextures.normal.get(), cmdList); numCustomSRVs++;

	shaderPipeline->setTexture2D("metalnessRoughnessAoBounceOne", m_gbufferTextures[2], cmdList); numCustomSRVs++;
	shaderPipeline->setTexture2D("metalnessRoughnessAoBounceTwo", m_outputTextures.metalnessRoughnessAO.get(), cmdList); numCustomSRVs++;

	shaderPipeline->setTexture2D("shadows", shadows, cmdList); numCustomSRVs++;
	shaderPipeline->setTexture2D("depthPositions", m_outputTextures.depthPositions.get(), cmdList); numCustomSRVs++;

	shaderPipeline->setTexture2D("brdfLUT", m_brdfTexture, cmdList); numCustomSRVs++;

	if (camera) {
		shaderPipeline->setCBufferVar("clipToView", &glm::inverse(camera->getProjMatrix()), sizeof(glm::mat4));
		shaderPipeline->setCBufferVar("viewToWorld", &glm::inverse(camera->getViewMatrix()), sizeof(glm::mat4));
		shaderPipeline->setCBufferVar("cameraPosition", &camera->getPosition(), sizeof(glm::vec3));
	}
	if (lightSetup) {
		auto& plData = lightSetup->getPointLightsData();
		shaderPipeline->setCBufferVar("pointLights", &plData, sizeof(plData));
	}

	static_cast<DX12Mesh*>(m_fullscreenModel->getMesh(0))->draw_new(*this, cmdList, 0, -numCustomSRVs);

	// setTexture2D transitions texture to pixel shader resource
	// We need to transition them back to a state that is usable on a compute queue before the next frame
	// TODO: batch transitions
	m_outputTextures.albedo->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_COPY_SOURCE);
	m_outputTextures.normal->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_COPY_SOURCE);
	m_outputTextures.metalnessRoughnessAO->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_COPY_SOURCE);
	shadows->transitionStateTo(cmdList, D3D12_RESOURCE_STATE_COPY_SOURCE);

	return m_shadedOuput.get();
}

void DX12RaytracingRenderer::begin(Camera* camera) {
	Renderer::begin(camera);
	m_metaballs.clear();
}

bool DX12RaytracingRenderer::onEvent(Event& event) {
	EventHandler::dispatch<WindowResizeEvent>(event, SAIL_BIND_EVENT(&DX12RaytracingRenderer::onResize));

	// Pass along events
	m_dxr.onEvent(event);
	return true;
}

void DX12RaytracingRenderer::submit(Mesh* mesh, const glm::mat4& modelMatrix, RenderFlag flags) {
	RenderCommand cmd;
	cmd.type = RENDER_COMMAND_TYPE_MODEL;
	cmd.model.mesh = mesh;
	cmd.transform = glm::transpose(modelMatrix);
	cmd.flags = flags;
	// Resize to match numSwapBuffers (specific to dx12)
	cmd.hasUpdatedSinceLastRender.resize(m_context->getNumGPUBuffers(), false);
	commandQueue.push_back(cmd);
}

void DX12RaytracingRenderer::submitMetaball(RenderCommandType type, Material* material, const glm::vec3& pos, RenderFlag flags) {
	assert(type != RenderCommandType::RENDER_COMMAND_TYPE_MODEL);

	if (type == RenderCommandType::RENDER_COMMAND_TYPE_NON_MODEL_METABALL) {
		DXRBase::Metaball ball;
		ball.pos = pos;
		ball.distToCamera = glm::length(ball.pos - camera->getPosition());

		m_metaballs.emplace_back(ball);
	}
}

void DX12RaytracingRenderer::submitDecal(const glm::vec3& pos, const glm::mat3& rot, const glm::vec3& halfSize) {
	DXRShaderCommon::DecalData decalData;
	decalData.position = pos;
	decalData.rot = rot;
	decalData.halfSize = halfSize;
	m_decals[m_currNumDecals % MAX_DECALS] = decalData;
	m_currNumDecals++;
}

void DX12RaytracingRenderer::submitWaterPoint(const glm::vec3& pos) {
	m_dxr.addWaterAtWorldPosition(pos);
}

void DX12RaytracingRenderer::updateMetaballAABB() {
	
}

void DX12RaytracingRenderer::setGBufferInputs(DX12RenderableTexture** inputs) {
	m_dxr.setGBufferInputs(inputs);
}

bool DX12RaytracingRenderer::onResize(WindowResizeEvent& event) {
	m_outputTextures.albedo->resize(event.getWidth(), event.getHeight());
	m_outputTextures.normal->resize(event.getWidth(), event.getHeight());
	m_outputTextures.metalnessRoughnessAO->resize(event.getWidth(), event.getHeight());
	m_outputTextures.shadows->resize(event.getWidth(), event.getHeight());
	m_shadowsLastFrame->resize(event.getWidth(), event.getHeight());
	return true;
}