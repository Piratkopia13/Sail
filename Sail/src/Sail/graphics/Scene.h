#pragma once

#include "../entities/Entity.h"
#include "camera/Camera.h"
#include "../events/Events.h"
//#include "postprocessing/PostProcessPipeline.h"
#include "Sail/graphics/geometry/PerUpdateRenderObject.h"

class LightSetup;
class Renderer;
class Model;
class PerUpdateRenderObject;

// TODO: make this class virtual and have the actual scene in the demo/game project
class Scene : public IEventListener {
public:
	Scene();
	~Scene();

	// Adds an entity to later be drawn
	// This takes shared ownership of the entity
	void addEntity(Entity::SPtr entity);

	// Add entity which won't change during runtime and therefore doesn't
	// need to be copied into a frame packet every tick.
	void addStaticEntity(Entity::SPtr staticEntity);

	void setLightSetup(LightSetup* lights);
	void draw(Camera& camera, const float alpha = 1.0f);


	void prepareUpdate();

	void prepareRenderObjects();

	virtual bool onEvent(Event& event) override;

private:
	bool onResize(WindowResizeEvent& event);

private:
	// Static object entities that don't need to be double buffered since they're not
	// modified/added/removed in update. They're just used in render.
	// Should consist of at least a ModelComponent and a StaticMatrixComponent.
	std::vector<Entity::SPtr> m_staticObjectEntities;


	// Dynamic objects are split into game objects and render objects to prevent
	// data races when rendering objects that have just been modified/deleted from
	// the scene.

	// Game objects are used for everything but the rendering pass
	//
	// Should include Model, Transform, Physics, Sound, etc.
	std::vector<Entity::SPtr> m_gameObjectEntities;


	// Render objects are essentially read-only and have only the minimum required data
	// needed to render the corresponding game object. A list of render objects is created
	// at the end of each CPU update from the m_GameObjectEntities.
	std::vector<PerUpdateRenderObject> m_dynamicRenderObjects[SNAPSHOT_BUFFER_SIZE];
	std::mutex m_perFrameLocks[SNAPSHOT_BUFFER_SIZE];


	std::unique_ptr<Renderer> m_renderer;
	//DeferredRenderer m_renderer;
	//std::unique_ptr<DX11RenderableTexture> m_deferredOutputTex;
	//PostProcessPipeline m_postProcessPipeline;

};