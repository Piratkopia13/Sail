#include "pch.h"
#include "RenderSystem.h"
#include "..//..//components/ModelComponent.h"
#include "..//..//components/TransformComponent.h"
#include "..//..//components/RealTimeComponent.h"
#include "..//..//components/BoundingBoxComponent.h"
#include "..//..//components/MetaballComponent.h"
#include "..//..//Entity.h"
#include "..//..//..//graphics/camera/Camera.h"
#include "..//..//..//graphics/geometry/Model.h"
#include "..//..//..//Application.h"


RenderSystem::RenderSystem() {

	registerComponent<ModelComponent>(false, true, false);
	registerComponent<MetaballComponent>(false, true, false);
	registerComponent<TransformComponent>(true, true, false);

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

		 if (model) {
			 if (entity->getComponent<RealTimeComponent>()) {
				 // If it's a real-time entity render with the most recent update
				 // Not that for these entities should be updated once per frame for this to work correctly
				 m_renderer->submit(model->getModel(), transform->getMatrix(),
					 (model->getModel()->isAnimated()) ? Renderer::MESH_DYNAMIC : Renderer::MESH_STATIC);
			 } else {
				 // If not interpolate between the two most recent updates
				 m_renderer->submit(model->getModel(), transform->getRenderMatrix(alpha),
					 (model->getModel()->isAnimated()) ? Renderer::MESH_DYNAMIC : Renderer::MESH_STATIC);
			 }
		 } else if (metaball) {
			 m_renderer->submitNonMesh(Renderer::RENDER_COMMAND_TYPE_NON_MODEL_METABALL, metaball->getMaterial(), transform->getRenderMatrix(alpha), Renderer::MESH_STATIC);
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
					 m_renderer->submit(wireframeModel, boundingBox->getTransform()->getMatrix(), 
						 Renderer::MESH_STATIC);
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

