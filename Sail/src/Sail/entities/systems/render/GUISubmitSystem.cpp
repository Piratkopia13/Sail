#include "pch.h"
#include "GUISubmitSystem.h"
#include "Sail/Application.h"
#include "Sail/entities/components/Components.h"


GUISubmitSystem::GUISubmitSystem() {
	registerComponent<GUIComponent>(true, false, false);
}

GUISubmitSystem::~GUISubmitSystem() {

}

void GUISubmitSystem::submitAll() {
	Renderer* renderer = Application::getInstance()->getRenderWrapper()->getScreenSpaceRenderer();
	for (auto& e : entities) {
		Renderer::RenderFlag flags = Renderer::MESH_STATIC;
		Model* model = e->getComponent<ModelComponent>()->getModel();
		if (e->hasComponent<ModelComponent>()) {
			renderer->submit(model, e->getComponent<TransformComponent>()->getMatrix(), flags);
		}

	}
}