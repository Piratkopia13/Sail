#include "pch.h"
#include "Renderer.h"
#include "Sail/graphics/geometry/Model.h"

void Renderer::begin(Camera* camera) {
	this->camera = camera;
	commandQueue.clear();
}

void Renderer::submit(Model* model, Material* material, const glm::mat4& modelMatrix) {
	for (unsigned int i = 0; i < model->getNumberOfMeshes(); i++) {
		submit(model->getMesh(i), material, modelMatrix);
	}
}

void Renderer::submit(Mesh* mesh, Material* material, const glm::mat4& modelMatrix) {
	commandQueue.emplace_back(mesh, material, glm::transpose(modelMatrix));
}

void Renderer::setLightSetup(LightSetup* lightSetup) {
	this->lightSetup = lightSetup;
}

void Renderer::end() {

}
