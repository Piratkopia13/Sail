#include "pch.h"
#include "Scene.h"
#include "../entities/components/Components.h"
#include "geometry/Model.h"
#include "light/LightSetup.h"
#include "../utils/Utils.h"
#include "Sail/Application.h"
#include "Sail/api/Renderer.h"
#include "Environment.h"
#include "material/OutlineMaterial.h"


Scene::Scene()  {
	m_renderer = std::unique_ptr<Renderer>(Renderer::Create(Renderer::FORWARD));

	// Set up the environment
	m_environment = std::make_unique<Environment>();
	addEntity(m_environment->getSkyboxEntity());
}

Scene::~Scene() { }

void Scene::addEntity(Entity::SPtr entity) {
	m_entities.emplace_back(entity);
}

void Scene::draw(Camera& camera) {
	SAIL_PROFILE_FUNCTION();

	LightSetup lightSetup;
	m_renderer->setLightSetup(&lightSetup);

	m_renderer->begin(&camera, m_environment.get());

	{
		SAIL_PROFILE_SCOPE("Submit models");

		// Submit outline meshes first since they have to be drawn before the mesh they outline
		//for (Entity::SPtr& entity : m_entities) {
		//	if (entity->isSelectedInGui()) {
		//		auto model = entity->getComponent<ModelComponent>();
		//		auto transform = entity->getComponent<TransformComponent>();
		//		// TODO: allow meshes to be rendered using any shader
		//		if (model && transform)
		//			m_renderer->submit(model->getModel().get(), &m_outlineMaterial, transform->getMatrix());
		//	}
		//}

		for (Entity::SPtr& entity : m_entities) {

			// Add all lights to the lightSetup
			auto plComp = entity->getComponent<PointLightComponent>();
			if (plComp) lightSetup.addPointLight(plComp.get());
			auto dlComp = entity->getComponent<DirectionalLightComponent>();
			if (dlComp) lightSetup.setDirectionalLight(dlComp.get());

			auto model = entity->getComponent<ModelComponent>();
			auto transform = entity->getComponent<TransformComponent>();
			//if (!transform)	Logger::Error("Tried to draw entity that is missing a TransformComponent!");

			Material* material = nullptr;
			// Material is not required for rendering and may be set to nullptr - this is a lie
			if (auto materialComp = entity->getComponent<MaterialComponent<>>())
				material = materialComp->get();

				
			if (model && transform && material) {
				m_renderer->submit(model->getModel().get(), material, transform->getMatrix());
				entity->setIsBeingRendered(true);
			} else {
				// Indicate that the entity is not being rendered for some reason, this may be shown in the gui
				entity->setIsBeingRendered(false);
			}
		}
	}

	m_renderer->end();
	m_renderer->present();

	m_renderer->setLightSetup(nullptr);
}

std::vector<Entity::SPtr>& Scene::getEntites() {
	return m_entities;
}

Environment* Scene::getEnvironment() {
	return m_environment.get();
}
