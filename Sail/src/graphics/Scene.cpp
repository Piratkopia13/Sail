#include "Scene.h"
#include "../entities/components/Components.h"
#include "geometry/Model.h"
#include "light/LightSetup.h"

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
		TransformComponent* transform = entity->getComponent<TransformComponent>();
		if (!transform)	Logger::Error("Tried to draw entity that is missing a TransformComponent!");
		ModelComponent* model = entity->getComponent<ModelComponent>();
		if (!model)	Logger::Error("Tried to draw entity that is missing a ModelComponent!");

		m_renderer.submit(model->getModel(), transform->getTransform().getMatrix());
	}

	m_renderer.end();
	m_renderer.present();

}
