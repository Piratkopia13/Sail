#include "pch.h"
#include "HitboxSubmitSystem.h"
#include "..//..//components/BoundingBoxComponent.h"
#include "..//..//components/TransformComponent.h"
#include "..//..//Entity.h"
#include "..//..//..//Application.h"

HitboxSubmitSystem::HitboxSubmitSystem() : m_renderHitBoxes(false) {
	registerComponent<BoundingBoxComponent>(true, true, false);
	registerComponent<TransformComponent>(true, true, false);
}

HitboxSubmitSystem::~HitboxSubmitSystem() {
}

void HitboxSubmitSystem::toggleHitboxes() {
	m_renderHitBoxes = !m_renderHitBoxes;
}

void HitboxSubmitSystem::submitAll() {
	if (m_renderHitBoxes) {
		Renderer* renderer = Application::getInstance()->getRenderWrapper()->getCurrentRenderer();
		for (auto& e : entities) {
			if (Model* wireframeModel = e->getComponent<BoundingBoxComponent>()->getWireframeModel()) {
				renderer->submit(wireframeModel, e->getComponent<TransformComponent>()->getMatrix(), Renderer::MESH_STATIC);
			}
		}
	}
}
