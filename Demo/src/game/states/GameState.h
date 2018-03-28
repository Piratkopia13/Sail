#pragma once

#include "Sail.h"

class GameState : public State {
public:
	GameState(StateStack& stack);
	~GameState();

	// Process input for the state
	virtual bool processInput(float dt);
	// Process window resizing for the state
	virtual bool resize(int width, int height);
	// Updates the state
	virtual bool update(float dt);
	// Renders the state
	virtual bool render(float dt);

private:
	Application* m_app;
	// Camera
	PerspectiveCamera m_cam;
	FlyingCameraController m_camController;
	bool m_flyCam;

	// Scene
	//std::unique_ptr<Scene> m_scene;

	ForwardRenderer m_renderer;
	LightSetup m_lights;

	// Texts
	SailFont m_font;
	Text m_fpsText;
	Text m_debugCamText;

	std::unique_ptr<Model> m_cubeModel;

};