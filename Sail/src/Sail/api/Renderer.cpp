#include "pch.h"
#include "Renderer.h"
#include "Sail/graphics/geometry/Model.h"

void Renderer::begin(Camera* camera) {
	this->camera = camera;
	commandQueue.clear();
}

void Renderer::submit(Model* model, const glm::mat4& modelMatrix, RenderFlag flags, glm::vec3 teamColor) {
	for (unsigned int i = 0; i < model->getNumberOfMeshes(); i++) {
		submit(model->getMesh(i), modelMatrix, flags, teamColor);
	}
}

void Renderer::submit(Mesh* mesh, const glm::mat4& modelMatrix, RenderFlag flags, glm::vec3 teamColor) {
	RenderCommand cmd;
	cmd.type = RENDER_COMMAND_TYPE_MODEL;
	cmd.model.mesh = mesh;
	cmd.transform = glm::transpose(modelMatrix);
	cmd.flags = flags;
	cmd.teamColor = teamColor;
	commandQueue.push_back(cmd);
}

void Renderer::setLightSetup(LightSetup* lightSetup) {
	this->lightSetup = lightSetup;
}
