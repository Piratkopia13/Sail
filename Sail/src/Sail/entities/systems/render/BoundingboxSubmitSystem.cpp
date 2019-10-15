#include "pch.h"
#include "BoundingboxSubmitSystem.h"
#include "..//..//components/BoundingBoxComponent.h"
#include "..//..//components/CullingComponent.h"
#include "..//..//Entity.h"
#include "..//..//..//Application.h"

BoundingboxSubmitSystem::BoundingboxSubmitSystem() : m_renderHitBoxes(false) {
	registerComponent<BoundingBoxComponent>(true, true, false);
}

BoundingboxSubmitSystem::~BoundingboxSubmitSystem() {
}

void BoundingboxSubmitSystem::toggleHitboxes() {
	m_renderHitBoxes = !m_renderHitBoxes;
}

void BoundingboxSubmitSystem::submitAll() {
	if (m_renderHitBoxes) {
		Renderer* renderer = Application::getInstance()->getRenderWrapper()->getCurrentRenderer();
		for (auto& e : entities) {
			BoundingBoxComponent* boundingBox = e->getComponent<BoundingBoxComponent>();
			if (Model* wireframeModel = boundingBox->getWireframeModel()) {
				Renderer::RenderFlag flags = Renderer::MESH_STATIC;
				CullingComponent* culling = e->getComponent<CullingComponent>();
				if (!culling || (culling && culling->isVisible)) {
					flags |= Renderer::IS_VISIBLE_ON_SCREEN;
				}
				renderer->submit(wireframeModel, boundingBox->getTransform()->getMatrix(), flags);
			}
		}
	}
}
