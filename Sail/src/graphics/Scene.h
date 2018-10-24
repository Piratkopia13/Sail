#pragma once

#include "../entities/Entity.h"
#include "renderer/ForwardRenderer.h"
#include "renderer/DeferredRenderer.h"
#include "camera/Camera.h"
#include "../events/Events.h"
#include "postprocessing/PostProcessPass.h"

class LightSetup;

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
	std::vector<Entity::Ptr> m_entities;
	//ForwardRenderer m_renderer;
	DeferredRenderer m_renderer;
	std::unique_ptr<RenderableTexture> m_deferredOutputTex;
	PostProcessPass m_postProcessPass;

};