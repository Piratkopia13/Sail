#include "pch.h"
#include "DX11ForwardRenderer.h"
#include "Sail/api/shader/PipelineStateObject.h"
#include "Sail/graphics/light/LightSetup.h"
#include "Sail/api/shader/Shader.h"
#include "Sail/Application.h"
#include "../DX11API.h"
#include "DX11DeferredRenderer.h"

Renderer* Renderer::Create(Renderer::Type type) {
	switch (type) {
	case FORWARD:
		return new DX11ForwardRenderer();
	case DEFERRED:
		return new DX11DeferredRenderer();
	default:
		Logger::Error("Tried to create a renderer of unknown or unimplemented type: " + std::to_string(type));
	}
	return nullptr;
}

DX11ForwardRenderer::DX11ForwardRenderer() {
	m_context = Application::getInstance()->getAPI<DX11API>();
}

DX11ForwardRenderer::~DX11ForwardRenderer() { }

void* DX11ForwardRenderer::present(Renderer::RenderFlag flags, void* skippedPrepCmdList) {
	auto& resman = Application::getInstance()->getResourceManager();

	if (!(flags & Renderer::RenderFlag::SkipPreparation)) {
		m_context->renderToBackBuffer();
		// Clear back buffer
		m_context->clear({ 0.1f, 0.2f, 0.3f, 1.0f });
	}
	if (!(flags & Renderer::RenderFlag::SkipRendering)) {
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
	return nullptr;
}

void DX11ForwardRenderer::useDepthBuffer(void* buffer, void* cmdList) {
	auto* dsv = static_cast<ID3D11DepthStencilView*>(buffer);
	m_context->getDeviceContext()->OMSetRenderTargets(1, m_context->getBackBufferRTV(), dsv);
}

