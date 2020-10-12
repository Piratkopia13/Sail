#include "pch.h"
#include "Scene.h"
#include "../entities/Entity.h"
#include "../entities/components/Components.h"
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
	m_environment = std::make_unique<Environment>(this);
}

Scene::~Scene() { }

Entity Scene::createEntity(const std::string& name) {
	Entity e = { m_registry.create(), this };
	e.addComponent<NameComponent>(name);
	e.addComponent<IsBeingRenderedComponent>(false);
	e.addComponent<IsSelectedComponent>(false);
	return e;
}

void Scene::destroyEntity(Entity& entity) {
	// Handle relations to the entity that is about to be destroyed
	auto relation = entity.tryGetComponent<RelationshipComponent>();
	if (relation) {
		if (relation->prev) {
			Entity(relation->prev, this).getComponent<RelationshipComponent>().next = relation->next;
		}
		if (relation->next) {
			Entity(relation->next, this).getComponent<RelationshipComponent>().prev = relation->prev;
		}
		if (relation->parent) {
			auto& parentEnt = Entity(relation->parent, this);
			auto& parentRel = parentEnt.getComponent<RelationshipComponent>();
			if (entity == parentRel.first) {
				parentRel.first = relation->next;
			}
		}

		// Remove children recursively
		auto curr = Entity(relation->first, this);
		for (size_t i = 0; i < relation->numChildren; i++) {
			// Store a copy of the ID to the next entity, since the relation component will get destroyed
			auto& rel = curr.getComponent<RelationshipComponent>();
			auto next = rel.next;
			destroyEntity(curr);

			curr = Entity(next, this);
		}
	}

	// Finally destroy the entity
	m_registry.destroy(entity);
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
		auto view = m_registry.view<SkyboxComponent>();
		for (auto entity : view) {
			auto skybox = view.get<SkyboxComponent>(entity);
			auto mesh = m_registry.get<MeshComponent>(entity);
			auto material = m_registry.get<MaterialComponent>(entity);
			auto transform = m_registry.get<TransformComponent>(entity);

			m_forwardRenderer->submit(mesh.get(), material.get()->getShader(Renderer::FORWARD), material.get(), transform.getMatrix());
		}
	}
#endif

	// Add all lights to the lightSetup
	{
		auto plView = m_registry.view<PointLightComponent>();
		for (auto entity : plView) {
			Entity e(entity, this);
			
			auto plComp = plView.get<PointLightComponent>(entity);
			lightSetup.addPointLight(&plComp);
		}

		auto dlView = m_registry.view<DirectionalLightComponent>();
		for (auto entity : dlView) {
			Entity e(entity, this);

			auto dlComp = dlView.get<DirectionalLightComponent>(entity);
			lightSetup.setDirectionalLight(&dlComp);
		}
	}

	// Drawing of meshes
	{
		SAIL_PROFILE_SCOPE("Submit models");

		auto view = m_registry.view<TransformComponent>(entt::exclude<SkyboxComponent>);
		for (auto entity : view) {
			glm::mat4 parentTransform(1.0f);

			Entity e(entity, this);
			auto* relation = e.tryGetComponent<RelationshipComponent>();
			// Don't draw entities with parents, since these have already been drawn
			if (!relation || !relation->parent) {
				submitEntity(e, doDXR, parentTransform);
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

Environment* Scene::getEnvironment() {
	return m_environment.get();
}

void Scene::submitEntity(Entity& entity, bool doDXR, const glm::mat4& parentTransform) {
	auto* outlineShader = &Application::getInstance()->getResourceManager().getShaderSet(Shaders::OutlineShader);

	auto* mesh = entity.tryGetComponent<MeshComponent>();
	auto* transform = entity.tryGetComponent<TransformComponent>();
	glm::mat4 transformMatrix = parentTransform;
	if (transform) {
		transformMatrix *= transform->getMatrix();
	}

	// Submit a copy for rendering as outline if selected in gui
	if (entity.isSelected() && mesh && mesh->get() && transform)
		m_forwardRenderer->submit(mesh->get(), outlineShader, &m_outlineMaterial, transformMatrix);

	Material* material = nullptr;
	if (auto* materialComp = entity.tryGetComponent<MaterialComponent>())
		material = materialComp->get();

	if (mesh && mesh->get() && transform && material) {
#if USE_DEFERRED
		if (doDXR) {
			// Submit all to the ray-tracer
			m_raytracingRenderer->submit(mesh->get(), nullptr, material, transformMatrix);
		}
#endif 

		// Submit to deferred or forward depending on the material
		Shader* shader = nullptr;
#if USE_DEFERRED
		if (shader = material->getShader(Renderer::DEFERRED))
			m_deferredRenderer->submit(mesh->get(), shader, material, transformMatrix);
		else
#endif 
		{
			if (shader = material->getShader(Renderer::FORWARD))
				m_forwardRenderer->submit(mesh->get(), shader, material, transformMatrix);
		}

		entity.setIsBeingRendered(true);
	} else {
		// Indicate that the entity is not being rendered for some reason, this may be shown in the GUI
		entity.setIsBeingRendered(false);
	}


	// Draw children recursively
	auto* relation = entity.tryGetComponent<RelationshipComponent>();
	if (relation) { // Don't draw entities with parents, since these have already been drawn
		auto curr = Entity(relation->first, this);
		for (size_t i = 0; i < relation->numChildren; ++i) {
			submitEntity(curr, doDXR, transformMatrix); // Recursive call

			auto rel = curr.getComponent<RelationshipComponent>();
			curr = Entity(rel.next, this);
		}
	}
}

entt::registry& Scene::getEnttRegistry() {
	return m_registry;
}
