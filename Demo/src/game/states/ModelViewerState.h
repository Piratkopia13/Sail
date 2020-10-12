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

	EditorGui m_editorGui;
	EntitiesGui m_entitiesGui;

	// Camera
	PerspectiveCamera m_cam;
	FlyingCameraController m_camController;

	Scene m_scene;

};