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
	auto& cmd = commandQueue.emplace_back();
	cmd.mesh = mesh;
	cmd.shader = shader;
	cmd.material = material;
	cmd.transform = glm::transpose(modelMatrix);
	cmd.dxrFlags = MESH_STATIC;
}

void Renderer::setLightSetup(LightSetup* lightSetup) {
	this->lightSetup = lightSetup;
}

void Renderer::useDepthBuffer(void* buffer, void* cmdList) { }

void* Renderer::getDepthBuffer() {
	return nullptr;
}

void Renderer::end() { }
