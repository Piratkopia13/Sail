#pragma once

#include "Sail.h"

class GameState : public State {
public:
	GameState(StateStack& stack);
	~GameState();

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
	// Camera
	PerspectiveCamera m_cam;
	FlyingCameraController m_camController;

	Entity m_texturedCubeEntity;
	std::vector<Entity> m_transformTestEntities;
	Scene m_scene;
};