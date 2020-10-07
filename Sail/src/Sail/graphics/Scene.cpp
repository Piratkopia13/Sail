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

#define USE_DEFERRED 1
#define ENABLE_SKYBOX 1

Scene::Scene()  {
#if USE_DEFERRED
	m_deferredRenderer = std::unique_ptr<Renderer>(Renderer::Create(Renderer::DEFERRED));
	if (Application::getInstance()->getSettings().getBool(Settings::Graphics_DXR)) {
		// Raytracing renderer has to be created after the deferred renderer since it uses resources created by the deferred renderers constructor
		m_raytracingRenderer = std::unique_ptr<Renderer>(Renderer::Create(Renderer::RAYTRACED));
	}
#endif
	m_forwardRenderer = std::unique_ptr<Renderer>(Renderer::Create(Renderer::FORWARD));


	// Set up the environment
	m_environment = std::make_unique<Environment>();
}

Scene::~Scene() { }

void Scene::addEntity(Entity::SPtr entity) {
	m_entities.emplace_back(entity);
}

void Scene::draw(Camera& camera) {
	SAIL_PROFILE_FUNCTION();

#if USE_DEFERRED
	bool doDXR = Application::getInstance()->getSettings().getBool(Settings::Graphics_DXR);
	if (!m_raytracingRenderer && doDXR) {
		// Handle enabling of DXR in runtime
		m_raytracingRenderer = std::unique_ptr<Renderer>(Renderer::Create(Renderer::RAYTRACED));
	} else if (m_raytracingRenderer && !doDXR) {
		// Handle disabling of DXR in runtime
		Application::getInstance()->getAPI()->waitForGPU();
		m_raytracingRenderer.reset();
	}
#endif


	LightSetup lightSetup;
	m_forwardRenderer->setLightSetup(&lightSetup);
#if USE_DEFERRED
	m_deferredRenderer->setLightSetup(&lightSetup);
	if (doDXR)
		m_raytracingRenderer->setLightSetup(&lightSetup);
#endif
	
	// Begin default pass
#if USE_DEFERRED
	if (doDXR)
		m_raytracingRenderer->begin(&camera, m_environment.get());
#endif
	m_forwardRenderer->begin(&camera, m_environment.get());
#if USE_DEFERRED
	m_deferredRenderer->begin(&camera, m_environment.get());
#endif
#if ENABLE_SKYBOX
	// Draw skybox first for transparency to work with it
	{
		auto e = m_environment->getSkyboxEntity();
		auto model = e->getComponent<ModelComponent>();
		auto transform = e->getComponent<TransformComponent>();
		auto material = e->getComponent<MaterialComponent<>>();
		m_forwardRenderer->submit(model->getModel().get(), material->get()->getShader(Renderer::FORWARD), material->get(), transform->getMatrix());
	}
#endif

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
			if (entity->isSelectedInGui() && model && model->getModel() && transform)
				m_forwardRenderer->submit(model->getModel().get(), outlineShader, &m_outlineMaterial, transform->getMatrix());

			Material* material = nullptr;
			if (auto materialComp = entity->getComponent<MaterialComponent<>>())
				material = materialComp->get();

			if (model && model->getModel() && transform && material) {
#if USE_DEFERRED
				if (doDXR) {
					// Submit all to the raytracer
					m_raytracingRenderer->submit(model->getModel().get(), nullptr, material, transform->getMatrix());
				}
#endif 

				// Submit to deferred or forward depending on the material
				Shader* shader = nullptr;
#if USE_DEFERRED
				if (shader = material->getShader(Renderer::DEFERRED))
					m_deferredRenderer->submit(model->getModel().get(), shader, material, transform->getMatrix());
				else
#endif 
				{
					if (shader = material->getShader(Renderer::FORWARD))
						m_forwardRenderer->submit(model->getModel().get(), shader, material, transform->getMatrix());
				}
				
				entity->setIsBeingRendered(true);
			} else {
				// Indicate that the entity is not being rendered for some reason, this may be shown in the gui
				entity->setIsBeingRendered(false);
			}
		}
	}
#if USE_DEFERRED
	m_deferredRenderer->end();
#endif
	m_forwardRenderer->end();
#if USE_DEFERRED
	if (doDXR)
		m_raytracingRenderer->end();

	// Execution order is important
	// Run geometry pass and ssao
	void* cmdList = m_deferredRenderer->present(Renderer::SkipDeferredShading | Renderer::SkipExecution);
	if (doDXR) {
		// Run raytracing (which uses the geometry pass output)
		m_raytracingRenderer->present(Renderer::SkipPreparation | Renderer::SkipExecution, cmdList);
	}
	// Run deferred shading
	m_deferredRenderer->present(Renderer::SkipPreparation | Renderer::SkipRendering | Renderer::SkipExecution, cmdList);
	// Run forward pass and execute everything
	m_forwardRenderer->useDepthBuffer(m_deferredRenderer->getDepthBuffer(), cmdList);
	m_forwardRenderer->present(Renderer::SkipPreparation, cmdList); // Execute deferred and forward default pass
#else
	m_forwardRenderer->present(Renderer::Default);
#endif

#if USE_DEFERRED
	m_deferredRenderer->setLightSetup(nullptr);
	if (doDXR)
		m_raytracingRenderer->setLightSetup(nullptr);
#endif
	m_forwardRenderer->setLightSetup(nullptr);
}

std::vector<Entity::SPtr>& Scene::getEntites() {
	return m_entities;
}

Environment* Scene::getEnvironment() {
	return m_environment.get();
}
