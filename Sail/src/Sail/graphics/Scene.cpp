#include "pch.h"
#include "Scene.h"
#include "../entities/components/Components.h"
#include "geometry/Model.h"
#include "light/LightSetup.h"
#include "../utils/Utils.h"
#include "Sail/Application.h"
#include "Sail/api/Renderer.h"


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
	m_entities.push_back(entity);
}

void Scene::setLightSetup(LightSetup* lights) {
	m_renderer->setLightSetup(lights);
}

void Scene::draw(Camera& camera) {
	SAIL_PROFILE_FUNCTION();

	m_renderer->begin(&camera);

	{
		SAIL_PROFILE_SCOPE("Submit models");

		for (Entity::SPtr& entity : m_entities) {
			ModelComponent* model = entity->getComponent<ModelComponent>();
			if (model) {
				TransformComponent* transform = entity->getComponent<TransformComponent>();
				if (!transform)	Logger::Error("Tried to draw entity that is missing a TransformComponent!");

				m_renderer->submit(model->getModel(), transform->getMatrix());
			}
		}
	}

	m_renderer->end();
	m_renderer->present();
	//m_renderer->present(m_deferredOutputTex.get());

	//m_postProcessPipeline.run(*m_deferredOutputTex, nullptr);

	// Draw text last
	// TODO: sort entity list instead of iterating entire list twice
	for (Entity::SPtr& entity : m_entities) {
		TextComponent* text = entity->getComponent<TextComponent>();
		if (text) {
			text->draw();
		}
	}
}