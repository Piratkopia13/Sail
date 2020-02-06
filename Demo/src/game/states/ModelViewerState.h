#pragma once

#include "Sail.h"
#include "../modelViewer/ModelViewerGui.h"

class ModelViewerState : public State {
public:
	ModelViewerState(StateStack& stack);
	~ModelViewerState();

	// Process input for the state
	virtual bool processInput(float dt) override;
	// Updates the state
	virtual bool update(float dt) override;
	// Renders the state
	virtual bool render(float dt) override;
	// Renders imgui
	virtual bool renderImgui(float dt) override;

private:
	Application* m_app;

	ModelViewerGui m_viewerGui;

	// Camera
	PerspectiveCamera m_cam;
	FlyingCameraController m_camController;

	Entity::SPtr m_texturedCubeEntity;
	std::vector<Entity::SPtr> m_transformTestEntities;

	Scene m_scene;
	LightSetup m_lights;
	std::unique_ptr<Environment> m_environment;

	std::unique_ptr<Model> m_planeModel;

};