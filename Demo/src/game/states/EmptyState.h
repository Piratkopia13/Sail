#pragma once

#include "Sail.h"

/*
* This state can be used for testing while implementing a new graphics API
* No textures, models or anything will be loaded or rendered
*/

class EmptyState : public State {
public:
	EmptyState(StateStack& stack);
	~EmptyState();

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
	std::unique_ptr<Renderer> m_forwardRenderer;
	std::shared_ptr<Model> m_model;
	PerspectiveCamera m_cam;
	FlyingCameraController m_camController;

};