#include "pch.h"
#include "BoundingboxSubmitSystem.h"
#include "Sail/entities/components/BoundingBoxComponent.h"
#include "Sail/entities/components/RagdollComponent.h"
#include "Sail/entities/components/CullingComponent.h"
#include "Sail/entities/Entity.h"
#include "Sail/Application.h"

BoundingboxSubmitSystem::BoundingboxSubmitSystem() : m_renderHitBoxes(false) {
	registerComponent<BoundingBoxComponent>(true, true, false);
	registerComponent<RagdollComponent>(false, true, false);
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
				Renderer::RenderFlag flags = Renderer::MESH_STATIC | Renderer::HIDE_IN_DXR;
				CullingComponent* culling = e->getComponent<CullingComponent>();
				if (!culling || (culling && culling->isVisible)) {
					flags |= Renderer::IS_VISIBLE_ON_SCREEN;
				}
				renderer->submit(wireframeModel, boundingBox->getTransform()->getMatrixWithUpdate(), flags, 0);
			}

			RagdollComponent* ragdollComp = e->getComponent<RagdollComponent>();
			if (ragdollComp && ragdollComp->wireframeModel) {
				Renderer::RenderFlag flags = Renderer::MESH_STATIC | Renderer::HIDE_IN_DXR;
				CullingComponent* culling = e->getComponent<CullingComponent>();
				if (!culling || (culling && culling->isVisible)) {
					flags |= Renderer::IS_VISIBLE_ON_SCREEN;
				}
				for (size_t i = 0; i < ragdollComp->contactPoints.size(); i++) {
					renderer->submit(ragdollComp->wireframeModel, ragdollComp->contactPoints[i].transform.getMatrixWithUpdate(), flags, 0);
				}
			}
		}
	}
}
