#include "pch.h"
#include "DX11ForwardRenderer.h"
#include "Sail/api/shader/PipelineStateObject.h"
#include "Sail/graphics/light/LightSetup.h"
#include "Sail/api/shader/Shader.h"
#include "Sail/Application.h"
#include "../DX11API.h"
//#include "DX11DeferredRenderer.h"
#include "../shader/DX11Shader.h"

Renderer* Renderer::Create(Renderer::Type type) {
	switch (type) {
	case FORWARD:
		return new DX11ForwardRenderer();
	//case DEFERRED:
		//return new DX11DeferredRenderer();
	default:
		Logger::Error("Tried to create a renderer of unknown or unimplemented type: " + std::to_string(type));
	}
	return nullptr;
}

DX11ForwardRenderer::DX11ForwardRenderer() {
	m_context = Application::getInstance()->getAPI<DX11API>();
}

DX11ForwardRenderer::~DX11ForwardRenderer() { }

void* DX11ForwardRenderer::present(Renderer::PresentFlag flags, void* skippedPrepCmdList) {
	auto& resman = Application::getInstance()->getResourceManager();

	if (!(flags & Renderer::PresentFlag::SkipPreparation)) {
		m_context->renderToBackBuffer();
		// Clear back buffer
		m_context->clear({ 0.1f, 0.2f, 0.3f, 1.0f });
	}
	if (!(flags & Renderer::PresentFlag::SkipRendering)) {
		// Iterate unique PSO's
		for (auto it : commandQueue) {
			PipelineStateObject* pso = it.first;
			auto& renderCommands = it.second;
			DX11Shader* shader = static_cast<DX11Shader*>(pso->getShader());

			shader->updateDescriptorsAndMaterialIndices(renderCommands, *environment, pso, skippedPrepCmdList);

			pso->bind();

			if (camera) {
				shader->trySetCBufferVar("sys_mView", &camera->getViewMatrix(), sizeof(glm::mat4), skippedPrepCmdList);
				shader->trySetCBufferVar("sys_mProjection", &camera->getProjMatrix(), sizeof(glm::mat4), skippedPrepCmdList);
				shader->trySetCBufferVar("sys_mVP", &camera->getViewProjection(), sizeof(glm::mat4), skippedPrepCmdList);
				shader->trySetCBufferVar("sys_cameraPos", &camera->getPosition(), sizeof(glm::vec3), skippedPrepCmdList);
			}

			if (lightSetup) {
				auto& [dlData, dlDataByteSize] = lightSetup->getDirLightData();
				auto& [plData, plDataByteSize] = lightSetup->getPointLightsData();
				shader->trySetCBufferVar("dirLight", dlData, dlDataByteSize, skippedPrepCmdList);
				shader->trySetCBufferVar("pointLights", plData, plDataByteSize, skippedPrepCmdList);
			}

			// Sort based on distance to draw back to front (for transparency)
			// TODO: Only sort meshes that have transparency
			// TODO: Fix ordering between different PSO's
			std::sort(renderCommands.begin(), renderCommands.end(), [&](Renderer::RenderCommand& a, Renderer::RenderCommand& b) {
				float dstA = glm::distance2(glm::vec3(glm::transpose(a.transform)[3]), camera->getPosition());
				float dstB = glm::distance2(glm::vec3(glm::transpose(b.transform)[3]), camera->getPosition());
				return dstA > dstB;
			});

			for (RenderCommand& command : renderCommands) {
				shader->trySetConstantVar("sys_materialIndex", &command.materialIndex, sizeof(unsigned int), skippedPrepCmdList);
				shader->trySetConstantVar("sys_mWorld", &glm::transpose(command.transform), sizeof(glm::mat4), skippedPrepCmdList);
				command.mesh->draw(*this, command.material, shader, environment);
			}
		}
	}
	return nullptr;
}

void DX11ForwardRenderer::useDepthBuffer(void* buffer, void* cmdList) {
	auto* dsv = static_cast<ID3D11DepthStencilView*>(buffer);
	m_context->getDeviceContext()->OMSetRenderTargets(1, m_context->getBackBufferRTV(), dsv);
}

