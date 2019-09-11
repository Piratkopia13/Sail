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
	bool renderImguiConsole(float dt);

private:
	Application* m_app;
	// Camera
	PerspectiveCamera m_cam;
	FlyingCameraController m_camController;

	Entity::SPtr m_texturedCubeEntity;
	std::vector<Entity::SPtr> m_transformTestEntities;

	const std::string createCube(const glm::vec3& position);

	Scene m_scene;
	LightSetup m_lights;
	ConsoleCommands m_cc;
	std::unique_ptr<Model> m_cubeModel;
	std::unique_ptr<Model> m_planeModel;

};