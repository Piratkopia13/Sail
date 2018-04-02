#include "Scene.h"
#include "../entities/components/Components.h"
#include "geometry/Model.h"
#include "light/LightSetup.h"
#include "../utils/Utils.h"

Scene::Scene() {

}

Scene::~Scene() {

}

void Scene::addEntity(Entity::Ptr entity) {
	m_entities.push_back(MOVE(entity));
}

void Scene::setLightSetup(LightSetup* lights) {
	m_renderer.setLightSetup(lights);
}

void Scene::draw(Camera& camera) {

	m_renderer.begin(&camera);

	for (Entity::Ptr& entity : m_entities) {
		ModelComponent* model = entity->getComponent<ModelComponent>();
		if (model) {
			TransformComponent* transform = entity->getComponent<TransformComponent>();
			if (!transform)	Logger::Error("Tried to draw entity that is missing a TransformComponent!");

			m_renderer.submit(model->getModel(), transform->getTransform().getMatrix());
		}
		TextComponent* text = entity->getComponent<TextComponent>();
		if (text) {
			text->draw();
		}
	}

	m_renderer.end();
	m_renderer.present();

}
