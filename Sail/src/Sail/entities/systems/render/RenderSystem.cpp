#include "pch.h"
#include "RenderSystem.h"
#include "..//..//components/ModelComponent.h"
#include "..//..//components/TransformComponent.h"
#include "..//..//components/RealTimeComponent.h"
#include "..//..//Entity.h"
#include "..//..//..//graphics/camera/Camera.h"
#include "..//..//..//graphics/geometry/Model.h"
#include "..//..//..//Application.h"


RenderSystem::RenderSystem() {
	requiredComponentTypes.push_back(ModelComponent::ID);
	requiredComponentTypes.push_back(TransformComponent::ID);

	m_renderer = Application::getInstance()->getRenderWrapper()->getCurrentRenderer();
}

RenderSystem::~RenderSystem() {
}

 void RenderSystem::update(float dt) {

}

 void RenderSystem::draw(Camera& camera, const float alpha) {

	 ModelComponent* mc = nullptr;
	 for (auto& entity : entities) {
		 mc = entity->getComponent<ModelComponent>();

		 ModelComponent* model = entity->getComponent<ModelComponent>();
		 TransformComponent* transform = entity->getComponent<TransformComponent>();

		 if (entity->getComponent<RealTimeComponent>()) {
			 // If it's a real-time entity render with the most recent update
			 // Not that for these entities should be updated once per frame for this to work correctly
			 m_renderer->submit(model->getModel(), transform->getMatrix(), 
				 (model->getModel()->isAnimated()) ? Renderer::MESH_DYNAMIC : Renderer::MESH_STATIC);
		 }
		 else {
			 // If not interpolate between the two most recent updates
			 m_renderer->submit(model->getModel(), transform->getRenderMatrix(alpha),
				 (model->getModel()->isAnimated()) ? Renderer::MESH_DYNAMIC : Renderer::MESH_STATIC);
		 }

		 // Solution for bounding boxes

	 } // for each entity
	 
	 m_renderer->end();
	 m_renderer->present((false) ? Application::getInstance()->getRenderWrapper()->getPostProcessPipeline() : nullptr);

 }

 void RenderSystem::draw(void) {
	 m_renderer->begin(nullptr);
	 m_renderer->end();
	 m_renderer->present();
 }

