#include "pch.h"
#include "DX11ForwardRenderer.h"
#include "Sail/api/shader/ShaderPipeline.h"
#include "Sail/graphics/light/LightSetup.h"

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
		ShaderPipeline* shader = command.mesh->getMaterial()->getShader();
		shader->bind(nullptr);

		shader->setCBufferVar("sys_mWorld", &glm::transpose(command.transform), sizeof(glm::mat4));
		shader->setCBufferVar("sys_mVP", &camera->getViewProjection(), sizeof(glm::mat4));
		shader->setCBufferVar("sys_cameraPos", &camera->getPosition(), sizeof(glm::vec3));

		if (lightSetup) {
			auto& dlData = lightSetup->getDirLightData();
			auto& plData = lightSetup->getPointLightsData();
			shader->setCBufferVar("dirLight", &dlData, sizeof(dlData));
			shader->setCBufferVar("pointLights", &plData, sizeof(plData));
		}

		command.mesh->draw(*this);
	}
}


