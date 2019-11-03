#include "pch.h"
#include "ModelSubmitSystem.h"
#include "..//..//components/ModelComponent.h"
#include "..//..//components/TransformComponent.h"
#include "..//..//components/CullingComponent.h"
#include "..//..//..//Application.h"
#include "..//..//Entity.h"

ModelSubmitSystem::ModelSubmitSystem() {
	registerComponent<ModelComponent>(true, true, false);
	registerComponent<TransformComponent>(true, true, false);
	registerComponent<CullingComponent>(false, true, false);
}

ModelSubmitSystem::~ModelSubmitSystem() {
}

void ModelSubmitSystem::submitAll(const float alpha) {
	Renderer* renderer = Application::getInstance()->getRenderWrapper()->getCurrentRenderer();
	for (auto& e : entities) {
		auto* model = e->getComponent<ModelComponent>();
		auto* transform = e->getComponent<TransformComponent>();
		auto* culling = e->getComponent<CullingComponent>();

		Renderer::RenderFlag flags = (model->getModel()->isAnimated()) ? Renderer::MESH_DYNAMIC : Renderer::MESH_STATIC;
		
		if ((!culling || (culling && culling->isVisible)) && model->renderToGBuffer) {
			flags |= Renderer::IS_VISIBLE_ON_SCREEN;
		}
		renderer->submit(model->getModel(), transform->getRenderMatrix(alpha), transform->getRenderMatrixLastFrame(), flags);
	}
}
