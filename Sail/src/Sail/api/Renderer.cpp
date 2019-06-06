#include "pch.h"
#include "Renderer.h"
#include "Sail/graphics/geometry/Model.h"

void Renderer::begin(Camera* camera) {
	this->camera = camera;
	commandQueue.clear();
}

void Renderer::submit(Model* model, const glm::mat4& modelMatrix) {
	for (unsigned int i = 0; i < model->getNumberOfMeshes(); i++) {
		submit(model->getMesh(i), modelMatrix);
	}
}

void Renderer::submit(Mesh* mesh, const glm::mat4& modelMatrix) {
	RenderCommand cmd;
	cmd.mesh = mesh;
	cmd.transform = glm::transpose(modelMatrix);
	commandQueue.push_back(cmd);
}

void Renderer::setLightSetup(LightSetup* lightSetup) {
	this->lightSetup = lightSetup;
}

void Renderer::end() {

}
