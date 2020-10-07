#include "pch.h"
#include "Renderer.h"
#include "Sail/graphics/geometry/Model.h"
#include "../Application.h"

Renderer::Renderer()
	: m_resman(&Application::getInstance()->getResourceManager())
{}

void Renderer::begin(Camera* camera, Environment* environment) {
	this->camera = camera;
	this->environment = environment;
	commandQueue.clear();
	commandQueueCustom.clear();
}

void Renderer::submit(Model* model, Shader* shader, Material* material, const glm::mat4& modelMatrix) {
	for (unsigned int i = 0; i < model->getNumberOfMeshes(); i++) {
		submit(model->getMesh(i), shader, material, modelMatrix);
	}
}

void Renderer::submit(Mesh* mesh, Shader* shader, Material* material, const glm::mat4& modelMatrix) {

	std::vector<RenderCommand>* cmdVector;
	
	if (shader) {
		// Find a matching pipelineStateObject used here to sort render commands
		PipelineStateObject* pso = &m_resman->getPSO(shader, mesh);
		auto it = commandQueue.find(pso);
		if (it == commandQueue.end()) {
			// PSO not already queued, do it
			cmdVector = &commandQueue.insert({ pso, {} }).first->second;
		} else {
			cmdVector = &it->second;
		}
	} else {
		// No shader was specified, this is valid for custom renderers, such as raytracing, that only use the geometry in the render commands
		// Use a separate unsorted vector for these
		cmdVector = &commandQueueCustom;
	}

	auto& cmd = cmdVector->emplace_back();
	cmd.mesh = mesh;
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
