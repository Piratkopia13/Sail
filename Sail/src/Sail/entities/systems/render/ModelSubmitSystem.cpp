#include "pch.h"
#include "ModelSubmitSystem.h"
#include "..//..//components/ModelComponent.h"
#include "..//..//components/TransformComponent.h"
#include "..//..//..//Application.h"
#include "..//..//Entity.h"

ModelSubmitSystem::ModelSubmitSystem() {
	registerComponent<ModelComponent>(true, true, false);
	registerComponent<TransformComponent>(true, true, false);
}

ModelSubmitSystem::~ModelSubmitSystem() {
}

void ModelSubmitSystem::submitAll(const float alpha) {
	Renderer* renderer = Application::getInstance()->getRenderWrapper()->getCurrentRenderer();
	for (auto& e : entities) {
		ModelComponent* model = e->getComponent<ModelComponent>();
		TransformComponent* transform = e->getComponent<TransformComponent>();
		renderer->submit(model->getModel(), transform->getRenderMatrix(alpha),
			(model->getModel()->isAnimated()) ? Renderer::MESH_DYNAMIC : Renderer::MESH_STATIC);
	}
}
