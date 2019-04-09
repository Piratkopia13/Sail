#pragma once

#include "../entities/Entity.h"
#include "renderer/ForwardRenderer.h"
#include "renderer/DeferredRenderer.h"
#include "camera/Camera.h"
#include "../events/Events.h"
#include "postprocessing/PostProcessPipeline.h"

class LightSetup;
// TODO: make this class virtual and have the actual scene in the demo/game project
class Scene : public IEventListener {
public:
	Scene();
	~Scene();

	// Adds an entity to later be drawn
	// This takes ownership of the entity
	void addEntity(Entity::Ptr entity);
	void setLightSetup(LightSetup* lights);
	void draw(Camera& camera);

	virtual void onEvent(Event& event) override;

private:
	bool onResize(WindowResizeEvent& event);

private:
	std::vector<Entity::Ptr> m_entities;
	//ForwardRenderer m_renderer;
	DeferredRenderer m_renderer;
	std::unique_ptr<DX11RenderableTexture> m_deferredOutputTex;
	PostProcessPipeline m_postProcessPipeline;

};