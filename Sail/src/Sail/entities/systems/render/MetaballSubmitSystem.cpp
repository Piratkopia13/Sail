#include "pch.h"
#include "MetaballSubmitSystem.h"
#include "..//..//..//Application.h"
#include "..//..//components/Components.h"
#include "..//..//Entity.h"

MetaballSubmitSystem::MetaballSubmitSystem() {
	registerComponent<MetaballComponent>(true, false, false);	// Data is not read, but the component is required for this system anyway
	registerComponent<TransformComponent>(true, true, false);
	registerComponent<CullingComponent>(false, true, false);
}

MetaballSubmitSystem::~MetaballSubmitSystem() {
}

void MetaballSubmitSystem::submitAll(const float alpha) {
	Renderer* renderer = Application::getInstance()->getRenderWrapper()->getCurrentRenderer();

	for (auto& e : entities) {
		TransformComponent* transform = e->getComponent<TransformComponent>();
		CullingComponent* culling = e->getComponent<CullingComponent>();

		Renderer::RenderFlag flags = Renderer::MESH_STATIC;
		if (!culling || (culling && culling->isVisible)) {
			flags |= Renderer::IS_VISIBLE_ON_SCREEN;
		}

		renderer->submitMetaball(Renderer::RENDER_COMMAND_TYPE_NON_MODEL_METABALL, nullptr, transform->getInterpolatedTranslation(alpha), flags);
	}
}
