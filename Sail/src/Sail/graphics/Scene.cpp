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
#include "../KeyCodes.h"


Scene::Scene()  {
	m_deferredRenderer = std::unique_ptr<Renderer>(Renderer::Create(Renderer::DEFERRED));
	m_forwardRenderer = std::unique_ptr<Renderer>(Renderer::Create(Renderer::FORWARD));
	m_raytracingRenderer = std::unique_ptr<Renderer>(Renderer::Create(Renderer::RAYTRACED));

	// Set up the environment
	m_environment = std::make_unique<Environment>();
}

Scene::~Scene() { }

void Scene::addEntity(Entity::SPtr entity) {
	m_entities.emplace_back(entity);
}

void Scene::draw(Camera& camera) {
	SAIL_PROFILE_FUNCTION();

	LightSetup lightSetup;
	m_forwardRenderer->setLightSetup(&lightSetup);
	m_deferredRenderer->setLightSetup(&lightSetup);
	m_raytracingRenderer->setLightSetup(&lightSetup);
	
	// Begin default pass
	m_raytracingRenderer->begin(&camera, m_environment.get());
	m_forwardRenderer->begin(&camera, m_environment.get());
	m_deferredRenderer->begin(&camera, m_environment.get());
	auto* outlineShader = &Application::getInstance()->getResourceManager().getShaderSet(Shaders::OutlineShader);
	// Drawing of meshes
	{
		SAIL_PROFILE_SCOPE("Submit models");

		for (Entity::SPtr& entity : m_entities) {
			// Add all lights to the lightSetup
			auto plComp = entity->getComponent<PointLightComponent>();
			if (plComp) lightSetup.addPointLight(plComp.get());
			auto dlComp = entity->getComponent<DirectionalLightComponent>();
			if (dlComp) lightSetup.setDirectionalLight(dlComp.get());

			auto model = entity->getComponent<ModelComponent>();
			auto transform = entity->getComponent<TransformComponent>();

			// Submit a copy for rendering as outline if selected in gui
			if (entity->isSelectedInGui() && model && transform)
				m_forwardRenderer->submit(model->getModel().get(), outlineShader, &m_outlineMaterial, transform->getMatrix());

			Material* material = nullptr;
			if (auto materialComp = entity->getComponent<MaterialComponent<>>())
				material = materialComp->get();

			if (model && transform && material) {
				// Submit all to the raytracer
				m_raytracingRenderer->submit(model->getModel().get(), nullptr, material, transform->getMatrix());

				// Submit to deferred or forward depending on the material
				Shader* shader = nullptr;
				if (shader = material->getShader(Renderer::DEFERRED))
					m_deferredRenderer->submit(model->getModel().get(), shader, material, transform->getMatrix());
				else if (shader = material->getShader(Renderer::FORWARD))
					m_forwardRenderer->submit(model->getModel().get(), shader, material, transform->getMatrix());
				
				entity->setIsBeingRendered(true);
			} else {
				// Indicate that the entity is not being rendered for some reason, this may be shown in the gui
				entity->setIsBeingRendered(false);
			}
		}
	}
	// Draw skybox last
	{
		auto e = m_environment->getSkyboxEntity();
		auto model = e->getComponent<ModelComponent>();
		auto transform = e->getComponent<TransformComponent>();
		auto material = e->getComponent<MaterialComponent<>>();
		m_forwardRenderer->submit(model->getModel().get(), material->get()->getShader(Renderer::FORWARD), material->get(), transform->getMatrix());
	}

	m_deferredRenderer->end();
	m_forwardRenderer->end();
	m_raytracingRenderer->end();

	// Raytracing test
	//if (Input::IsKeyPressed(SAIL_KEY_J))
		m_raytracingRenderer->present(Renderer::Default);

	void* cmdList = m_deferredRenderer->present(Renderer::SkipExecution);
	m_forwardRenderer->useDepthBuffer(m_deferredRenderer->getDepthBuffer(), cmdList);
	m_forwardRenderer->present(Renderer::SkipPreparation, cmdList); // Execute deferred and forward default pass

	m_deferredRenderer->setLightSetup(nullptr);
	m_forwardRenderer->setLightSetup(nullptr);
	m_raytracingRenderer->setLightSetup(nullptr);
}

std::vector<Entity::SPtr>& Scene::getEntites() {
	return m_entities;
}

Environment* Scene::getEnvironment() {
	return m_environment.get();
}
