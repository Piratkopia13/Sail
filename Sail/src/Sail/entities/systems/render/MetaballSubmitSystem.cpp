#include "pch.h"
#include "MetaballSubmitSystem.h"
#include "..//..//..//Application.h"
#include "..//..//components/Components.h"
#include "..//..//Entity.h"

template <typename T>
MetaballSubmitSystem<T>::MetaballSubmitSystem() {
	registerComponent<MetaballComponent>(true, true, false);
	registerComponent<TransformComponent>(true, true, false);
	registerComponent<CullingComponent>(false, true, false);
	registerComponent<T>(true, true, false);
}

template <typename T>
void MetaballSubmitSystem<T>::submitAll(const float alpha) {
	Renderer* renderer = Application::getInstance()->getRenderWrapper()->getCurrentRenderer();

	for (auto& e : entities) {
		TransformComponent* transform = e->getComponent<TransformComponent>();
		CullingComponent* culling = e->getComponent<CullingComponent>();

		Renderer::RenderFlag flags = Renderer::MESH_STATIC;
		if (!culling || (culling && culling->isVisible)) {
			flags |= Renderer::IS_VISIBLE_ON_SCREEN;
		}

		MetaballComponent* mc = e->getComponent<MetaballComponent>();
		renderer->submitMetaball(Renderer::RENDER_COMMAND_TYPE_NON_MODEL_METABALL, nullptr, transform->getInterpolatedTranslation(alpha), flags, mc->renderGroupIndex);
	}
}

template class MetaballSubmitSystem<RenderInActiveGameComponent>;
template class MetaballSubmitSystem<RenderInReplayComponent>;
