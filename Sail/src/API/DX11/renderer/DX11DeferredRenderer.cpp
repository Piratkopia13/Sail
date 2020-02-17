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

DX11DeferredRenderer::DX11DeferredRenderer() {
	m_context = Application::getInstance()->getAPI<DX11API>();

	auto windowWidth = Application::getInstance()->getWindow()->getWindowWidth();
	auto windowHeight = Application::getInstance()->getWindow()->getWindowHeight();

	for (int i = 0; i < NUM_GBUFFERS; i++) {
		m_gbufferTextures[i] = std::unique_ptr<DX11RenderableTexture>(static_cast<DX11RenderableTexture*>(
			RenderableTexture::Create(windowWidth, windowHeight, "GBuffer renderer output " + std::to_string(i), (i < 2) ? ResourceFormat::R16G16B16A16_FLOAT : ResourceFormat::R8G8B8A8, (i == 0))));
	}

	m_screenQuadModel = ModelFactory::ScreenQuadModel::Create();
}

DX11DeferredRenderer::~DX11DeferredRenderer() { }

void* DX11DeferredRenderer::present(Renderer::RenderFlag flags, void* skippedPrepCmdList) {
	if (!(flags & Renderer::RenderFlag::SkipPreparation))
		runFramePreparation();
	if (!(flags & Renderer::RenderFlag::SkipRendering)) {
		runGeometryPass();
		runShadingPass();
	}

	return nullptr;
}

void* DX11DeferredRenderer::getDepthBuffer() {
	return *m_gbufferTextures[0]->getDepthStencilView();
}

void DX11DeferredRenderer::runFramePreparation() {
	auto* devCon = m_context->getDeviceContext();
	
	// Bind gbuffers as render targets and clear them
	ID3D11RenderTargetView* rtvs[NUM_GBUFFERS];
	for (unsigned int i = 0; i < NUM_GBUFFERS; i++) {
		rtvs[i] = *m_gbufferTextures[i]->getRenderTargetView();
		m_gbufferTextures[i]->clear(glm::vec4(0.f, 0.f, 0.f, 1.0f));
	}
	devCon->OMSetRenderTargets(NUM_GBUFFERS, rtvs, *m_gbufferTextures[0]->getDepthStencilView());
	devCon->RSSetViewports(1, m_context->getViewport());
}

void DX11DeferredRenderer::runGeometryPass() {
	auto& resman = Application::getInstance()->getResourceManager();

	for (RenderCommand& command : commandQueue) {
		Shader* shader = command.shader;

		// Find a matching pipelineStateObject and bind it
		auto& pso = resman.getPSO(shader, command.mesh);
		pso.bind();

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

		command.mesh->draw(*this, command.material, shader, environment);
	}
}

void DX11DeferredRenderer::runShadingPass() {
	auto& resman = Application::getInstance()->getResourceManager();
	
	m_context->renderToBackBuffer();
	
	auto* shader = &Application::getInstance()->getResourceManager().getShaderSet(Shaders::DeferredShadingPassShader);
	auto* mesh = m_screenQuadModel->getMesh(0);

	// Find a matching pipelineStateObject and bind it
	auto& pso = resman.getPSO(shader, mesh);
	pso.bind();

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

	mesh->draw(*this, &m_shadingPassMaterial, shader, environment);

	// Unbind gbuffers from the shader to be used as render targets next frame
	ID3D11ShaderResourceView* nullSrvs[NUM_GBUFFERS];
	for (unsigned int i = 0; i < NUM_GBUFFERS; i++)
		nullSrvs[i] = nullptr;
	// The starting slot must match the first def_ texture in ShadingPassShader
	// TODO: make this more generic
	m_context->getDeviceContext()->PSSetShaderResources(3, NUM_GBUFFERS, nullSrvs);
}

