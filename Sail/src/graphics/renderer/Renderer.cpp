#include "Renderer.h"
#include "../geometry/Model.h"

Renderer::Renderer() { }

Renderer::~Renderer() { }

void Renderer::submit(Model* model, const DirectX::SimpleMath::Matrix& modelMatrix) {
	for (unsigned int i = 0; i < model->getNumberOfMeshes(); i++) {
		submit(model->getMesh(i), modelMatrix);
	}
}

