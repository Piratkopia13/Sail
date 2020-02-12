#include "pch.h"
#include "DX11ForwardRenderer.h"
#include "Sail/api/shader/PipelineStateObject.h"
#include "Sail/graphics/light/LightSetup.h"
#include "Sail/api/shader/Shader.h"
#include "Sail/Application.h"

Renderer* Renderer::Create(Renderer::Type type) {
	switch (type) {
	case FORWARD:
		return new DX11ForwardRenderer();
	default:
		Logger::Error("Tried to create a renderer of unknown or unimplemented type: " + std::to_string(type));
	}
	return nullptr;
}

DX11ForwardRenderer::DX11ForwardRenderer() { }

DX11ForwardRenderer::~DX11ForwardRenderer() { }

void DX11ForwardRenderer::present(RenderableTexture* output) {
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


