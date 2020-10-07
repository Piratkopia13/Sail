#include "pch.h"
#include "DX11DeferredRenderer.h"
#include "Sail/api/shader/PipelineStateObject.h"
#include "Sail/graphics/light/LightSetup.h"
#include "Sail/api/shader/Shader.h"
#include "Sail/Application.h"
#include "../DX11API.h"
#include "Sail/graphics/geometry/factory/ScreenQuadModel.h"
#include "../resources/DX11RenderableTexture.h"
#include "Sail/graphics/Environment.h"
#include "../shader/DX11Shader.h"

DX11DeferredRenderer::DX11DeferredRenderer() {
	auto* app = Application::getInstance();
	m_context = app->getAPI<DX11API>();

	auto windowWidth = app->getWindow()->getWindowWidth();
	auto windowHeight = app->getWindow()->getWindowHeight();

	for (int i = 0; i < NUM_GBUFFERS; i++) {
		m_gbufferTextures[i] = std::unique_ptr<DX11RenderableTexture>(static_cast<DX11RenderableTexture*>(
			RenderableTexture::Create(windowWidth, windowHeight, RenderableTexture::USAGE_SAMPLING_ACCESS, "GBuffer renderer output " + std::to_string(i), (i < 2) ? ResourceFormat::R16G16B16A16_FLOAT : ResourceFormat::R8G8B8A8, (i == 0))));
	}

	m_screenQuadModel = ModelFactory::ScreenQuadModel::Create();

	// SSAO
	if (app->getSettings().getBool(Settings::Graphics_SSAO)) {
		m_ssao = std::make_unique<SSAO>();
	}
}

DX11DeferredRenderer::~DX11DeferredRenderer() { }

void* DX11DeferredRenderer::present(Renderer::PresentFlag flags, void* skippedPrepCmdList) {
	bool doSSAO = Application::getInstance()->getSettings().getBool(Settings::Graphics_SSAO);
	if (!m_ssao && doSSAO) {
		// Handle enabling of ssao in runtime
		m_ssao = std::make_unique<SSAO>();
	} else if (m_ssao && !doSSAO) {
		// Handle disabling of ssao in runtime
		m_ssao.reset();
	}

	if (!(flags & Renderer::PresentFlag::SkipPreparation))
		runFramePreparation();
	if (!(flags & Renderer::PresentFlag::SkipRendering)) {
		runGeometryPass();
		if (doSSAO)
			runSSAO();
	}
	if (!(flags & Renderer::PresentFlag::SkipDeferredShading))
		runShadingPass();

	return nullptr;
}

void* DX11DeferredRenderer::getDepthBuffer() {
	return m_gbufferTextures[0]->getDSV();
}

void DX11DeferredRenderer::runFramePreparation() {
	auto* devCon = m_context->getDeviceContext();
	
	// Bind gbuffers as render targets and clear them
	ID3D11RenderTargetView* rtvs[NUM_GBUFFERS];
	for (unsigned int i = 0; i < NUM_GBUFFERS; i++) {
		rtvs[i] = m_gbufferTextures[i]->getRTV();
		m_gbufferTextures[i]->clear(glm::vec4(0.f, 0.f, (i == 0) ? FLT_MAX : 0.f, 0.0f)); // The position buffer z is cleared to FLT_MAX to fix ssao issue on skybox background
	}
	devCon->OMSetRenderTargets(NUM_GBUFFERS, rtvs, m_gbufferTextures[0]->getDSV());
	devCon->RSSetViewports(1, m_context->getViewport());
}

void DX11DeferredRenderer::runGeometryPass() {
	auto& resman = Application::getInstance()->getResourceManager();

	assert(false && "Deprecated");
	//for (RenderCommand& command : commandQueue) {
	//	Shader* shader = command.shader;

	//	// Find a matching pipelineStateObject and bind it
	//	auto& pso = resman.getPSO(shader, command.mesh);
	//	pso.bind();

	//	shader->trySetCBufferVar("sys_mWorld", &glm::transpose(command.transform), sizeof(glm::mat4));
	//	shader->trySetCBufferVar("sys_mView", &camera->getViewMatrix(), sizeof(glm::mat4));
	//	shader->trySetCBufferVar("sys_mProjection", &camera->getProjMatrix(), sizeof(glm::mat4));
	//	shader->trySetCBufferVar("sys_mVP", &camera->getViewProjection(), sizeof(glm::mat4));
	//	shader->trySetCBufferVar("sys_cameraPos", &camera->getPosition(), sizeof(glm::vec3));

	//	if (lightSetup) {
	//		auto& [dlData, dlDataByteSize] = lightSetup->getDirLightData();
	//		auto& [plData, plDataByteSize] = lightSetup->getPointLightsData();
	//		shader->trySetCBufferVar("dirLight", dlData, dlDataByteSize);
	//		shader->trySetCBufferVar("pointLights", plData, plDataByteSize);
	//	}

	//	command.mesh->draw(*this, command.material, shader, environment);
	//}
}

void DX11DeferredRenderer::runSSAO() {
	auto& resman = Application::getInstance()->getResourceManager();
	auto* devCon = m_context->getDeviceContext();

	auto* ssaoRenderTarget = static_cast<DX11RenderableTexture*>(m_ssao->getRenderTargetTexture());
	ssaoRenderTarget->clear({ 0.0f, 0.0f, 0.0f, 0.0f });
	auto rtv = ssaoRenderTarget->getRTV();
	devCon->OMSetRenderTargets(1, &rtv, nullptr);
	D3D11_VIEWPORT viewport = { 0 };
	viewport.Width = m_ssao->getRenderTargetWidth();
	viewport.Height = m_ssao->getRenderTargetHeight();
	viewport.MaxDepth = 1.0f;

	devCon->RSSetViewports(1, &viewport);

	auto* shader = &Application::getInstance()->getResourceManager().getShaderSet(Shaders::SSAOShader);
	auto* mesh = m_screenQuadModel->getMesh(0);

	// Find a matching pipelineStateObject and bind it
	auto& pso = resman.getPSO(shader, mesh);
	pso.bind();

	shader->trySetCBufferVar("sys_mView", &camera->getViewMatrix(), sizeof(glm::mat4), nullptr);
	shader->trySetCBufferVar("sys_mProjection", &camera->getProjMatrix(), sizeof(glm::mat4), nullptr);
	auto& [kernelData, kernelDataSize] = m_ssao->getKernel();
	shader->trySetCBufferVar("kernel", kernelData, kernelDataSize, nullptr);
	auto& [noiseData, noiseDataSize] = m_ssao->getNoise();
	shader->trySetCBufferVar("noise", noiseData, noiseDataSize, nullptr);
	glm::vec2 ssaoSize(m_ssao->getRenderTargetWidth(), m_ssao->getRenderTargetHeight());
	shader->trySetCBufferVar("windowSize", &ssaoSize, sizeof(glm::vec2), nullptr);

	auto materialFunc = [&](Shader* shader, Environment* environment, void* cmdList) {
		// Bind GBuffer textures
		assert(false && "Deprecated");
		//shader->setRenderableTexture("def_positions", m_gbufferTextures[0].get(), cmdList);
		//shader->setRenderableTexture("def_worldNormals", m_gbufferTextures[1].get(), cmdList);
	};
	m_shadingPassMaterial.setBindFunc(materialFunc);

	mesh->draw(*this, &m_shadingPassMaterial, shader, environment);

	// Unbind render target to allow using it as input in compute shader
	ID3D11RenderTargetView* rTargets[1] = { nullptr };
	devCon->OMSetRenderTargets(1, rTargets, nullptr);

	// Blur ssao output
	auto& blurHorizontalShader = static_cast<DX11Shader&>(Application::getInstance()->getResourceManager().getShaderSet(Shaders::GaussianBlurHorizontalComputeShader));
	auto& blurVerticalShader = static_cast<DX11Shader&>(Application::getInstance()->getResourceManager().getShaderSet(Shaders::GaussianBlurVerticalComputeShader));

	auto& settingsHorizontal = blurHorizontalShader.getSettings().computeShaderSettings;
	auto& settingsVertical = blurVerticalShader.getSettings().computeShaderSettings;
	float textureSizeDiff = 1.f;

	// Dispatch horizontal blur pass
	{
		blurHorizontalShader.setCBufferVar("textureSizeDifference", &textureSizeDiff, sizeof(float), nullptr);

		blurHorizontalShader.setRenderableTextureUAV("inoutput", ssaoRenderTarget);
		
		auto& pso = Application::getInstance()->getResourceManager().getPSO(&blurHorizontalShader);
		pso.bind();

		unsigned int x = (unsigned int)glm::ceil(m_ssao->getRenderTargetWidth() * settingsHorizontal.threadGroupXScale);
		unsigned int y = (unsigned int)glm::ceil(m_ssao->getRenderTargetHeight() * settingsHorizontal.threadGroupYScale);
		unsigned int z = 1;
		devCon->Dispatch(x, y, z);

		// Unbind texture to allow setting it as RT later
		blurHorizontalShader.setRenderableTextureUAV("inoutput", nullptr);
	}

	// Dispatch vertical blur pass
	{
		m_ssaoShadingTexture = static_cast<DX11RenderableTexture*>(blurVerticalShader.getRenderableTexture("output"));
		m_ssaoShadingTexture->resize(m_ssao->getRenderTargetWidth(), m_ssao->getRenderTargetHeight());

		blurVerticalShader.setCBufferVar("textureSizeDifference", &textureSizeDiff, sizeof(float), nullptr);
		assert(false && "Deprecated");
		//blurVerticalShader.setRenderableTexture("input", ssaoRenderTarget);
		blurVerticalShader.setRenderableTextureUAV("output", m_ssaoShadingTexture);

		auto& pso = Application::getInstance()->getResourceManager().getPSO(&blurVerticalShader);
		pso.bind();

		unsigned int x = (unsigned int)glm::ceil(m_ssao->getRenderTargetWidth() * settingsVertical.threadGroupXScale);
		unsigned int y = (unsigned int)glm::ceil(m_ssao->getRenderTargetHeight() * settingsVertical.threadGroupYScale);
		unsigned int z = 1;
		devCon->Dispatch(x, y, z);

		// Unbind
		assert(false && "Deprecated");
		//blurVerticalShader.setRenderableTexture("input", nullptr);
		blurVerticalShader.setRenderableTextureUAV("output", nullptr);
	}

	//m_ssaoShadingTexture = ssaoRenderTarget;
}

void DX11DeferredRenderer::runShadingPass() {
	auto& resman = Application::getInstance()->getResourceManager();
	bool useSSAO = Application::getInstance()->getSettings().getBool(Settings::Graphics_SSAO);

	m_context->renderToBackBuffer();
	
	auto* shader = &Application::getInstance()->getResourceManager().getShaderSet(Shaders::DeferredShadingPassShader);
	auto* mesh = m_screenQuadModel->getMesh(0);

	// Find a matching pipelineStateObject and bind it
	auto& pso = resman.getPSO(shader, mesh);
	pso.bind();

	shader->trySetCBufferVar("sys_mViewInv", &glm::inverse(camera->getViewMatrix()), sizeof(glm::mat4), nullptr);
	shader->trySetCBufferVar("sys_cameraPos", &camera->getPosition(), sizeof(glm::vec3), nullptr);
	int useSSAOInt = (int)useSSAO;
	shader->trySetCBufferVar("useSSAO", &useSSAOInt, sizeof(int), nullptr);
	int useShadowTextureInt = 0;
	shader->trySetCBufferVar("useShadowTexture", &useShadowTextureInt, sizeof(int), nullptr);

	if (lightSetup) {
		auto& [dlData, dlDataByteSize] = lightSetup->getDirLightData();
		auto& [plData, plDataByteSize] = lightSetup->getPointLightsData();
		unsigned int numPLs = lightSetup->getNumPLs();
		shader->trySetCBufferVar("dirLight", dlData, dlDataByteSize, nullptr);
		shader->trySetCBufferVar("pointLights", plData, plDataByteSize, nullptr);
		shader->trySetCBufferVar("sys_numPLights", &numPLs, sizeof(unsigned int), nullptr);
	}

	auto materialFunc = [&](Shader* shader, Environment* environment, void* cmdList) {
		auto* brdfLutTexture = &Application::getInstance()->getResourceManager().getTexture("pbr/brdfLUT.tga");
		// Bind environment textures
		assert(false && "Deprecated");

		//shader->setTexture("sys_texBrdfLUT", brdfLutTexture, cmdList);
		//shader->setTexture("irradianceMap", environment->getIrradianceTexture(), cmdList);
		//shader->setTexture("radianceMap", environment->getRadianceTexture(), cmdList);

		//// Bind GBuffer textures
		//shader->setRenderableTexture("def_positions", m_gbufferTextures[0].get(), cmdList);
		//shader->setRenderableTexture("def_worldNormals", m_gbufferTextures[1].get(), cmdList);
		//shader->setRenderableTexture("def_albedo", m_gbufferTextures[2].get(), cmdList);
		//shader->setRenderableTexture("def_mrao", m_gbufferTextures[3].get(), cmdList);
		//if (useSSAO)
		//	shader->setRenderableTexture("tex_ssao", m_ssaoShadingTexture, cmdList);
	};
	m_shadingPassMaterial.setBindFunc(materialFunc);

	mesh->draw(*this, &m_shadingPassMaterial, shader, environment);

	// Unbind gbuffers and ssao from the shader to be used as render targets next frame
	// +1 for the ssao input texture
	ID3D11ShaderResourceView* nullSrvs[NUM_GBUFFERS+1];
	for (unsigned int i = 0; i < NUM_GBUFFERS+1; i++)
		nullSrvs[i] = nullptr;
	// The starting slot must match the first def_ texture in ShadingPassShader
	// TODO: make this more generic
	m_context->getDeviceContext()->PSSetShaderResources(3, NUM_GBUFFERS+1, nullSrvs);
}

