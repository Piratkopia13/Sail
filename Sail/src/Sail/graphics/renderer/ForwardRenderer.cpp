#include "pch.h"
#include "ForwardRenderer.h"
#include "../shader/ShaderSet.h"
#include "../light/LightSetup.h"

ForwardRenderer::ForwardRenderer() {

}

ForwardRenderer::~ForwardRenderer() {

}

void ForwardRenderer::begin(Camera* camera) {
	m_camera = camera;

	commandQueue.clear();
}

void ForwardRenderer::submit(Mesh* mesh, const glm::mat4& modelMatrix) {
	RenderCommand cmd;
	cmd.mesh = mesh;
	cmd.transform = glm::transpose(modelMatrix);
	commandQueue.push_back(cmd);
	//Logger::Log("Submitted model at: " + Utils::vec3ToStr(commandQueue.back().modelMatrix.Transpose().Translation()));

}

void ForwardRenderer::submit(Model* model, const glm::mat4& modelMatrix) {
	Renderer::submit(model, modelMatrix);
}

void ForwardRenderer::setLightSetup(LightSetup* lightSetup) {
	m_lightSetup = lightSetup;
}

void ForwardRenderer::end() {
	/*for (RenderCommand& command : commandQueue) {
		Logger::Log("Preparing model at: " + Utils::vec3ToStr(command.modelMatrix.Transpose().Translation()));
	}*/
}

void ForwardRenderer::present(RenderableTexture* output) {

	for (RenderCommand& command : commandQueue) {
		ShaderSet* shader = command.mesh->getMaterial()->getShader();
		shader->bind();

		shader->setCBufferVar("sys_mWorld", &command.transform, sizeof(glm::mat4));
		shader->setCBufferVar("sys_mVP", &glm::transpose(m_camera->getViewProjection()), sizeof(glm::mat4));
		shader->setCBufferVar("sys_cameraPos", &m_camera->getPosition(), sizeof(glm::vec3));

		if (m_lightSetup) {
			auto& dlData = m_lightSetup->getDirLightData();
			auto& plData = m_lightSetup->getPointLightsData();
			shader->setCBufferVar("dirLight", &dlData, sizeof(dlData));
			shader->setCBufferVar("pointLights", &plData, sizeof(plData));
		}

		command.mesh->draw(*this);
	}
}
