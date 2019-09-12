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
	void draw(Camera& camera);

	virtual bool onEvent(Event& event) override;
	void changeRenderer(unsigned int index);

private:
	bool onResize(WindowResizeEvent& event);

private:
	std::vector<Entity::SPtr> m_entities;
	std::unique_ptr<Renderer> m_rendererRaster;
	std::unique_ptr<Renderer> m_rendererRaytrace;
	std::unique_ptr<Renderer>* m_currentRenderer;
	//DeferredRenderer m_renderer;
	//std::unique_ptr<DX11RenderableTexture> m_deferredOutputTex;
	//PostProcessPipeline m_postProcessPipeline;

};