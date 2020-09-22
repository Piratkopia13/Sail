#pragma once

#include "../entities/Entity.h"
#include "camera/Camera.h"
#include "material/OutlineMaterial.h"

class Renderer;
class Environment;

class Scene {
public:
	Scene();
	~Scene();

	// Adds an entity to later be drawn
	// This takes shared ownership of the entity
	void addEntity(Entity::SPtr entity);
	void draw(Camera& camera);

	std::vector<Entity::SPtr>& getEntites();
	Environment* getEnvironment();

private:
	std::unique_ptr<Environment> m_environment;
	std::vector<Entity::SPtr> m_entities;
	//std::unique_ptr<Renderer> m_deferredRenderer;
	std::unique_ptr<Renderer> m_forwardRenderer;
	//std::unique_ptr<Renderer> m_raytracingRenderer;

	//OutlineMaterial m_outlineMaterial;
};