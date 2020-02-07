#include "pch.h"
#include "Scene.h"
#include "../entities/components/Components.h"
#include "geometry/Model.h"
#include "light/LightSetup.h"
#include "../utils/Utils.h"
#include "Sail/Application.h"
#include "Sail/api/Renderer.h"


Scene::Scene()  {
	m_renderer = std::unique_ptr<Renderer>(Renderer::Create(Renderer::FORWARD));
}

Scene::~Scene() {

}

void Scene::addEntity(Entity::SPtr entity) {
	m_entities.push_back(entity);
}

void Scene::setLightSetup(LightSetup* lights) {
	m_renderer->setLightSetup(lights);
}

void Scene::draw(Camera& camera, Environment* environment) {
	SAIL_PROFILE_FUNCTION();

	m_renderer->begin(&camera, environment);

	{
		SAIL_PROFILE_SCOPE("Submit models");

		for (Entity::SPtr& entity : m_entities) {
			ModelComponent* model = entity->getComponent<ModelComponent>();
			if (model) {
				TransformComponent* transform = entity->getComponent<TransformComponent>();
				if (!transform)	Logger::Error("Tried to draw entity that is missing a TransformComponent!");

				Material* material = nullptr;
				// Material is not required for rendering and may be set to nullptr
				if (MaterialComponent* materialComp = entity->getComponent<MaterialComponent>())
					material = materialComp->get();

				m_renderer->submit(model->getModel(), material, transform->getMatrix());
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

std::vector<Entity::SPtr>& Scene::getEntites() {
	return m_entities;
}
