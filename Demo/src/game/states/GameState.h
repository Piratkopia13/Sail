#pragma once

#include "Sail.h"

class GameState : public State {
public:
	GameState(StateStack& stack);
	~GameState();

	// Process input for the state
	virtual bool processInput(float dt) override;
	// Sends events to the state
	virtual bool onEvent(Event& event) override;
	// Updates the state
	virtual bool update(float dt) override;
	// Renders the state
	virtual bool render(float dt) override;
	// Renders imgui
	virtual bool renderImgui(float dt) override;

private:
	bool onResize(WindowResizeEvent& event);

private:
	Application* m_app;
	// Camera
	PerspectiveCamera m_cam;
	FlyingCameraController m_camController;
	bool m_flyCam;

	Entity::SPtr m_testEntity;

	Scene m_scene;
	LightSetup m_lights;

	// Texts
	Text* m_fpsText;
	Text* m_debugCamText;

	std::unique_ptr<Model> m_cubeModel;
	std::unique_ptr<Model> m_planeModel;



	// TRANSFORM TESTING
	std::vector<Entity::SPtr> m_transformTestEntities;
	void createTransformTest();
	void updateTransformTest(const float dt);




};