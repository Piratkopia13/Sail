#include "pch.h"
#include "Scene.h"
#include "../entities/components/Components.h"
#include "geometry/Model.h"
#include "light/LightSetup.h"
#include "../utils/Utils.h"
#include "Sail/Application.h"
#include "Sail/api/Renderer.h"
#include "Sail/entities/ECS.h"

Scene::Scene() 
	//: m_postProcessPipeline(m_renderer)
{
	m_renderer = std::unique_ptr<Renderer>(Renderer::Create(Renderer::FORWARD));

	// TODO: the following method ish
	//m_postProcessPipeline.add<FXAAStage>();
	/*m_postProcessPipeline.add<GaussianBlurStage>(1.f / 1.f);
	m_postProcessPipeline.add<GaussianBlurStage>(1.f / 1.5f);
	m_postProcessPipeline.add<GaussianBlurStage>(1.f / 2.f);*/
	//m_postProcessPipeline.add<FXAAStage>();

	auto window = Application::getInstance()->getWindow();
	UINT width = window->getWindowWidth();
	UINT height = window->getWindowHeight();

	//m_deferredOutputTex = std::unique_ptr<DX11RenderableTexture>(SAIL_NEW DX11RenderableTexture(1U, width, height, false));

}

Scene::~Scene() {

}

void Scene::addEntity(Entity::SPtr entity) {
	m_GameObjectEntities.push_back(entity);
}

void Scene::setLightSetup(LightSetup* lights) {
	m_renderer->setLightSetup(lights);
}


// TODO: REMOVE, won't be needed with separate render objects
// NEEDS TO RUN BEFORE EACH UPDATE
// Copies the game state from the previous tick 
void Scene::prepareUpdate() {
	for (auto e : m_GameObjectEntities) {
		GameTransformComponent* transform = e->getComponent<GameTransformComponent>();
		if (transform) { transform->prepareUpdate(); }
	}
}


// creates a vector of render objects corresponding to the current state of the game objects
// Should be done at the end of each update tick.
// TODO: move the update indexes out of transform
void Scene::prepareRenderObjects() {
	/*for (auto e : m_perFrameRenderObjects[0]) {
		ECS::Instance()->destroyEntity(e);
	}*/
	const UINT ind = Application::GetUpdateIndex();
	const UINT t = Application::GetRenderIndex();
	m_perFrameLocks[ind].lock();
	m_perFrameRenderObjects[ind].clear();

	size_t test = m_perFrameRenderObjects[ind].size();

	for (auto gameObject : m_GameObjectEntities) {
		GameTransformComponent* transform = gameObject->getComponent<GameTransformComponent>();
		ModelComponent* model = gameObject->getComponent<ModelComponent>();
		if (transform && model) {
			//auto e = ECS::Instance()->createEntity("RenderEntity"); // TODO? unique name
			////Model* model = modelComponent->getModel();
			//e->addComponent<ModelComponent>(model->getModel());
			//e->addComponent<RenderTransformComponent>(transform);
			//

			//RenderTransform* rf = SAIL_NEW RenderTransform(transform);

			//PerFrameRenderObject pfo = PerFrameRenderObject(model->getModel(), rf);

			// TODO: allocate RenderTransforms sequentially in memory
			m_perFrameRenderObjects[ind].push_back(RenderTransform(transform, model));
		}
	}
	m_perFrameLocks[ind].unlock();

}


// TODO: loop through renderEntities
void Scene::draw(Camera& camera, const float alpha) {
	m_renderer->begin(&camera);

	const UINT ind = Application::GetRenderIndex();

	m_perFrameLocks[ind].lock();


	for (RenderTransform& obj : m_perFrameRenderObjects[ind]) {
		//if (obj.m_model && obj.m_transform) {
			//RenderTransformComponent* transform = entity->getComponent<RenderTransformComponent>();
			//if (!obj.m_transform)	Logger::Error("Tried to draw entity that is missing a TransformComponent!");

			m_renderer->submit(obj.getModel(), obj.getMatrix(alpha));
		//}
	}
	m_perFrameLocks[ind].unlock();


	m_renderer->end();
	m_renderer->present();

	//m_postProcessPipeline.run(*m_deferredOutputTex, nullptr);

	// Draw text last
	// TODO: sort entity list instead of iterating entire list twice
	//for (Entity::SPtr& entity : m_GameObjectEntities) {
	//	TextComponent* text = entity->getComponent<TextComponent>();
	//	if (text) {
	//		text->draw();
	//	}
	//}
}

bool Scene::onEvent(Event& event) {
	EventHandler::dispatch<WindowResizeEvent>(event, SAIL_BIND_EVENT(&Scene::onResize));

	// Forward events
	m_renderer->onEvent(event);
	//m_postProcessPipeline.onEvent(event);

	return true;
}

bool Scene::onResize(WindowResizeEvent & event) {

	unsigned int width = event.getWidth();
	unsigned int height = event.getHeight();

	//m_deferredOutputTex->resize(width, height);

	return false;
}
