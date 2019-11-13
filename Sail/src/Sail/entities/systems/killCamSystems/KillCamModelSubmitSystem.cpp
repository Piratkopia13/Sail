#include "pch.h"
#include "KillCamModelSubmitSystem.h"
#include "..//..//components/ModelComponent.h"
#include "..//..//components/RealTimeComponent.h"
#include "..//..//components/ReplayTransformComponent.h"
#include "..//..//components/CullingComponent.h"
#include "..//..//..//Application.h"
#include "..//..//Entity.h"

KillCamModelSubmitSystem::KillCamModelSubmitSystem() {
	registerComponent<ModelComponent>(true, true, false);
	registerComponent<ReplayTransformComponent>(true, true, false);
	registerComponent<CullingComponent>(false, true, false);
}

KillCamModelSubmitSystem::~KillCamModelSubmitSystem() {
}

void KillCamModelSubmitSystem::submitAll(const float alpha) {
	Renderer* renderer = Application::getInstance()->getRenderWrapper()->getCurrentRenderer();
	for (auto& e : entities) {
		ModelComponent* model = e->getComponent<ModelComponent>();
		ReplayTransformComponent* transform = e->getComponent<ReplayTransformComponent>();
		CullingComponent* culling = e->getComponent<CullingComponent>();

		Renderer::RenderFlag flags = (model->getModel()->isAnimated()) ? Renderer::MESH_DYNAMIC : Renderer::MESH_STATIC;

		if ((!culling || (culling && culling->isVisible)) && model->renderToGBuffer) {
			flags |= Renderer::IS_VISIBLE_ON_SCREEN;
		}

		renderer->submit(
			model->getModel(),
			transform->getRenderMatrix(alpha),
			flags,
			model->teamColorID
		);
	}
}
