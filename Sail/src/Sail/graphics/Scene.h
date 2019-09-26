#pragma once

#include "../entities/Entity.h"
#include "camera/Camera.h"
#include "../events/Events.h"
#include "postprocessing/PostProcessPipeline.h"

class LightSetup;
class Renderer;
class Model;

// TODO: make this class virtual and have the actual scene in the demo/game project
class Scene : public IEventListener {
public:
	Scene();
	~Scene();

	// Adds an entity to later be drawn
	// This takes shared ownership of the entity
	void addEntity(Entity::SPtr entity);

	void setLightSetup(LightSetup* lights);
	Entity::SPtr getSceneEntityByName(std::string name);
	const std::vector<Entity::SPtr>& getGameObjectEntities()const;
	void showBoundingBoxes(bool val);
	void draw(void);
	void draw(Camera& camera, const float alpha = 1.0f);


	virtual bool onEvent(Event& event) override;
	void changeRenderer(unsigned int index);
	bool& getDoProcessing();
private:
	bool onResize(WindowResizeEvent& event);

private:
	
	// The objects in the scene that should be rendered
	std::vector<Entity::SPtr> m_sceneEntities;

	std::unique_ptr<Renderer> m_renderer;
	//DeferredRenderer m_renderer;
	//std::unique_ptr<DX11RenderableTexture> m_deferredOutputTex;
	//PostProcessPipeline m_postProcessPipeline;


	std::unique_ptr<Renderer> m_rendererRaster;
	std::unique_ptr<Renderer> m_rendererRaytrace;
	std::unique_ptr<Renderer>* m_currentRenderer;
	PostProcessPipeline m_postProcessPipeline;
	bool m_doPostProcessing;
	bool m_showBoundingBoxes;
};