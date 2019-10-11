#include "pch.h"
#include "HitboxSubmitSystem.h"
#include "..//..//components/BoundingBoxComponent.h"
#include "..//..//Entity.h"
#include "..//..//..//Application.h"

HitboxSubmitSystem::HitboxSubmitSystem() : m_renderHitBoxes(false) {
	registerComponent<BoundingBoxComponent>(true, true, false);
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
			BoundingBoxComponent* boundingBox = e->getComponent<BoundingBoxComponent>();
			if (Model* wireframeModel = boundingBox->getWireframeModel()) {
				renderer->submit(wireframeModel, boundingBox->getTransform()->getMatrix(), Renderer::MESH_STATIC);
			}
		}
	}
}
