#include "pch.h"
#include "DX11ForwardRenderer.h"
#include "Sail/api/shader/PipelineStateObject.h"
#include "Sail/graphics/light/LightSetup.h"
#include "Sail/graphics/shader/Shader.h"

Renderer* Renderer::Create(Renderer::Type type) {
	switch (type) {
	case FORWARD:
		return new DX11ForwardRenderer();
	default:
		Logger::Error("Tried to create a renderer of unknown or unimplemented type: " + type);
	}
	return nullptr;
}

DX11ForwardRenderer::DX11ForwardRenderer() { }

DX11ForwardRenderer::~DX11ForwardRenderer() { }

void DX11ForwardRenderer::present(RenderableTexture* output) {
	for (RenderCommand& command : commandQueue) {
		PipelineStateObject* shaderPipeline = command.mesh->getShader()->getPipeline();
		shaderPipeline->bind();

		shaderPipeline->trySetCBufferVar("sys_mWorld", &glm::transpose(command.transform), sizeof(glm::mat4));
		shaderPipeline->trySetCBufferVar("sys_mView", &camera->getViewMatrix(), sizeof(glm::mat4));
		shaderPipeline->trySetCBufferVar("sys_mProjection", &camera->getProjMatrix(), sizeof(glm::mat4));
		shaderPipeline->trySetCBufferVar("sys_mVP", &camera->getViewProjection(), sizeof(glm::mat4));
		shaderPipeline->trySetCBufferVar("sys_cameraPos", &camera->getPosition(), sizeof(glm::vec3));

		if (lightSetup) {
			auto& [dlData, dlDataByteSize] = lightSetup->getDirLightData();
			auto& [plData, plDataByteSize] = lightSetup->getPointLightsData();
			shaderPipeline->trySetCBufferVar("dirLight", dlData, dlDataByteSize);
			shaderPipeline->trySetCBufferVar("pointLights", plData, plDataByteSize);
		}

		command.mesh->draw(*this, command.material, environment);
	}
}


