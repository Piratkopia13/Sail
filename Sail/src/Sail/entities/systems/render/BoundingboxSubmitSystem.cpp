#include "pch.h"
#include "BoundingboxSubmitSystem.h"
#include "..//..//components/BoundingBoxComponent.h"
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
				renderer->submit(wireframeModel, boundingBox->getTransform()->getMatrix(), Renderer::MESH_STATIC);
			}
		}
	}
}
