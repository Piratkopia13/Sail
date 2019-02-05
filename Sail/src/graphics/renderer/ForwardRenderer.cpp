#include "pch.h"
#include "ForwardRenderer.h"
#include "../shader/ShaderSet.h"
#include "../light/LightSetup.h"

using namespace DirectX::SimpleMath;

ForwardRenderer::ForwardRenderer() {

}

ForwardRenderer::~ForwardRenderer() {

}

void ForwardRenderer::begin(Camera* camera) {
	m_camera = camera;

	commandQueue.clear();
}

void ForwardRenderer::submit(Mesh* mesh, const DirectX::SimpleMath::Matrix& modelMatrix) {
	RenderCommand cmd;
	cmd.mesh = mesh;
	cmd.transform = modelMatrix.Transpose();
	commandQueue.push_back(cmd);
	//Logger::Log("Submitted model at: " + Utils::vec3ToStr(commandQueue.back().modelMatrix.Transpose().Translation()));

}

void ForwardRenderer::submit(Model* model, const DirectX::SimpleMath::Matrix& modelMatrix) {
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

		shader->setCBufferVar("sys_mWorld", &command.transform, sizeof(Matrix));
		shader->setCBufferVar("sys_mVP", &m_camera->getViewProjection().Transpose(), sizeof(Matrix));
		shader->setCBufferVar("sys_cameraPos", &m_camera->getPosition(), sizeof(Vector3));

		if (m_lightSetup) {
			auto& dlData = m_lightSetup->getDirLightData();
			auto& plData = m_lightSetup->getPointLightsData();
			shader->setCBufferVar("dirLight", &dlData, sizeof(dlData));
			shader->setCBufferVar("pointLights", &plData, sizeof(plData));
		}

		command.mesh->draw(*this);
	}
}
