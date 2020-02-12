#include "pch.h"
#include "Renderer.h"
#include "Sail/graphics/geometry/Model.h"

void Renderer::begin(Camera* camera, Environment* environment) {
	this->camera = camera;
	this->environment = environment;
	commandQueue.clear();
}

void Renderer::submit(Model* model, Shader* shader, Material* material, const glm::mat4& modelMatrix) {
	for (unsigned int i = 0; i < model->getNumberOfMeshes(); i++) {
		submit(model->getMesh(i), shader, material, modelMatrix);
	}
}

void Renderer::submit(Mesh* mesh, Shader* shader, Material* material, const glm::mat4& modelMatrix) {
	commandQueue.emplace_back(mesh, shader, material, glm::transpose(modelMatrix));
}

void Renderer::setLightSetup(LightSetup* lightSetup) {
	this->lightSetup = lightSetup;
}

void Renderer::end() {

}
