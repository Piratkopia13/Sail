#include "pch.h"
#include "RealTimeModelSubmitSystem.h"
#include "..//..//components/ModelComponent.h"
#include "..//..//components/TransformComponent.h"
#include "..//..//components/RealTimeComponent.h"
#include "..//..//..//Application.h"
#include "..//..//Entity.h"

RealTimeModelSubmitSystem::RealTimeModelSubmitSystem() {
	registerComponent<ModelComponent>(true, true, false);
	registerComponent<TransformComponent>(true, true, false);
	registerComponent<RealTimeComponent>(true, false, false);	// Data is not read, but the component is required for this system anyway
}

RealTimeModelSubmitSystem::~RealTimeModelSubmitSystem() {
}

void RealTimeModelSubmitSystem::submitAll(const float alpha) {
	Renderer* renderer = Application::getInstance()->getRenderWrapper()->getCurrentRenderer();
	for (auto& e : entities) {
		ModelComponent* model = e->getComponent<ModelComponent>();
		TransformComponent* transform = e->getComponent<TransformComponent>();

		renderer->submit(model->getModel(), transform->getMatrix(), (model->getModel()->isAnimated() ? Renderer::MESH_DYNAMIC : Renderer::MESH_STATIC));
	}
}
