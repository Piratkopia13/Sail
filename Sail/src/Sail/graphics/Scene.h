#pragma once

#include "../entities/Entity.h"
#include "camera/Camera.h"
#include "../events/Events.h"
//#include "postprocessing/PostProcessPipeline.h"

class LightSetup;
class Renderer;
// TODO: make this class virtual and have the actual scene in the demo/game project
class Scene : public IEventListener {
public:
	Scene();
	~Scene();

	// Adds an entity to later be drawn
	// This takes shared ownership of the entity
	void addEntity(Entity::SPtr entity);
	void setLightSetup(LightSetup* lights);
	void draw(Camera& camera, const float alpha = 1.0f);


	void prepareUpdate();

	virtual bool onEvent(Event& event) override;

private:
	bool onResize(WindowResizeEvent& event);

private:
	std::vector<Entity::SPtr> m_entities; // game objects

	// should be a ring buffer or something
	// make it as small as possible
	std::vector<Entity::SPtr> m_perFrameRenderObjects[4]; // TODO



	std::unique_ptr<Renderer> m_renderer;
	//DeferredRenderer m_renderer;
	//std::unique_ptr<DX11RenderableTexture> m_deferredOutputTex;
	//PostProcessPipeline m_postProcessPipeline;

};