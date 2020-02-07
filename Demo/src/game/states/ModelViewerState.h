#pragma once

#include "Sail.h"
#include "../editor/EntitiesGui.h"
#include "../editor/EditorGui.h"

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

	EditorGui m_viewerGui;
	EntitiesGui m_entitiesGui;

	// Camera
	PerspectiveCamera m_cam;
	FlyingCameraController m_camController;

	Entity::SPtr m_texturedCubeEntity;
	std::vector<Entity::SPtr> m_transformTestEntities;

	Scene m_scene;
	LightSetup m_lights;
	std::unique_ptr<Environment> m_environment;

	std::shared_ptr<Model> m_planeModel;

};