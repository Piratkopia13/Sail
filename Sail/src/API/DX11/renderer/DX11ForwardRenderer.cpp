#include "pch.h"
#include "DX11ForwardRenderer.h"
#include "Sail/api/shader/ShaderPipeline.h"
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

DX11ForwardRenderer::DX11ForwardRenderer() {

}

DX11ForwardRenderer::~DX11ForwardRenderer() {

}

void DX11ForwardRenderer::present(RenderableTexture* output) {

	for (RenderCommand& command : commandQueue) {
		ShaderPipeline* shaderPipeline = command.mesh->getShader()->getPipeline();
		shaderPipeline->bind();

		shaderPipeline->setCBufferVar("sys_mWorld", &glm::transpose(command.transform), sizeof(glm::mat4));
		shaderPipeline->setCBufferVar("sys_mVP", &camera->getViewProjection(), sizeof(glm::mat4));
		shaderPipeline->setCBufferVar("sys_cameraPos", &camera->getPosition(), sizeof(glm::vec3));

		if (lightSetup) {
			auto& dlData = lightSetup->getDirLightData();
			auto& plData = lightSetup->getPointLightsData();
			shaderPipeline->setCBufferVar("dirLight", &dlData, sizeof(dlData));
			shaderPipeline->setCBufferVar("pointLights", &plData, sizeof(plData));
		}

		command.mesh->draw(*this, command.material);
	}
}


