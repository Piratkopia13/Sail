#pragma once

#include "camera/Camera.h"
#include "material/OutlineMaterial.h"

#include <entt.hpp>

class Renderer;
class Environment;
class Entity;

class Scene {
public:
	// Allows a few classes to access the entt registry
	friend class Entity;
	friend class EntitiesGui;

public:
	Scene();
	~Scene();

	// Creates an entity in the scene and returns a handle to it
	Entity Scene::createEntity(const std::string& name = "Unnamed");
	// Removes an entity with all its components from the scene
	// This also handles relation dependencies
	// All children entites will also be destroyed
	void Scene::destroyEntity(Entity& entity);
	void draw(Camera& camera);

	uint32_t getEntityCount() const;

	Environment* getEnvironment();

	entt::registry m_registry;
private:
	void submitEntity(Entity& entity, bool doDXR, const glm::mat4& parentTransform);
	entt::registry& getEnttRegistry();

private:
	std::unique_ptr<Environment> m_environment;
	std::unique_ptr<Renderer> m_deferredRenderer;
	std::unique_ptr<Renderer> m_forwardRenderer;
	std::unique_ptr<Renderer> m_raytracingRenderer;

	OutlineMaterial m_outlineMaterial;
};