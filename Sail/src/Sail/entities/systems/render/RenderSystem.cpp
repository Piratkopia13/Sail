#include "pch.h"
#include "RenderSystem.h"
#include "Sail/entities/components/Components.h"
#include "Sail/entities/Entity.h"
#include "Sail/graphics/camera/Camera.h"
#include "Sail/graphics/geometry/Model.h"
#include "Sail/Application.h"


RenderSystem::RenderSystem() {

	registerComponent<ModelComponent>(false, true, false);
	registerComponent<MetaballComponent>(false, true, false);
	registerComponent<TransformComponent>(true, true, false);
	//registerComponent<BoundingBoxComponent>(true, true, false);

	refreshRenderer();
	m_renderHitboxes = false;
}

RenderSystem::~RenderSystem() {
}

 void RenderSystem::update(float dt) {

}

 void RenderSystem::toggleHitboxes() {
	 if (m_renderHitboxes) {
		 m_renderHitboxes = false;
	 }
	 else {
		 m_renderHitboxes = true;
	 }
 }

 void RenderSystem::refreshRenderer() {
	 m_renderer = Application::getInstance()->getRenderWrapper()->getCurrentRenderer();
 }

 void RenderSystem::draw(Camera& camera, const float alpha) {

	 m_renderer->begin(&camera);

	 for (auto& entity : entities) {

		 ModelComponent* model = entity->getComponent<ModelComponent>();
		 MetaballComponent* metaball = entity->getComponent<MetaballComponent>();
		 TransformComponent* transform = entity->getComponent<TransformComponent>();
		 CullingComponent* cullComponent = entity->getComponent<CullingComponent>();

		 if (model) {
			Renderer::RenderFlag flags = (model->getModel()->isAnimated()) ? Renderer::MESH_DYNAMIC : Renderer::MESH_STATIC;
			if (!cullComponent || (cullComponent && cullComponent->isVisible)) {
				flags |= Renderer::IS_VISIBLE_ON_SCREEN;
			}
			 if (entity->getComponent<RealTimeComponent>()) {
				 // If it's a real-time entity render with the most recent update
				 // Not that for these entities should be updated once per frame for this to work correctly
				 m_renderer->submit(model->getModel(), transform->getMatrix(), flags);
			 } else {
				 // If not interpolate between the two most recent updates
				 m_renderer->submit(model->getModel(), transform->getRenderMatrix(alpha), flags);
			 }
		 } else if (metaball) {
			 Renderer::RenderFlag flags = Renderer::MESH_STATIC;
			 if (!cullComponent || (cullComponent && cullComponent->isVisible)) {
				 flags |= Renderer::IS_VISIBLE_ON_SCREEN;
			 }
			 m_renderer->submitNonMesh(Renderer::RENDER_COMMAND_TYPE_NON_MODEL_METABALL, nullptr, transform->getRenderMatrix(alpha), flags);
		 } else {
			 continue;
		 }

		 // Solution for bounding boxes
		 if (m_renderHitboxes) {
			 BoundingBoxComponent* boundingBox = entity->getComponent<BoundingBoxComponent>();
			 if (boundingBox) {
				Model* wireframeModel = boundingBox->getWireframeModel();
				if (wireframeModel) {
					// Bounding boxes are visualized with their most update since that's what's used for hit detection
					Renderer::RenderFlag flags = Renderer::MESH_STATIC;
					if (!cullComponent || (cullComponent && cullComponent->isVisible)) {
						flags |= Renderer::IS_VISIBLE_ON_SCREEN;
					}
					m_renderer->submit(wireframeModel, boundingBox->getTransform()->getMatrix(), flags);
				}
			 }
		 }

	 } // for each entity

	 m_renderer->end();
	 m_renderer->present((Application::getInstance()->getRenderWrapper()->getDoPostProcessing()
		 ) ? Application::getInstance()->getRenderWrapper()->getPostProcessPipeline() : nullptr);

 }

 void RenderSystem::draw(void) {
	 m_renderer->begin(nullptr);
	 m_renderer->end();
	 m_renderer->present();
 }

