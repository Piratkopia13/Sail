#pragma once

#include "Sail.h"
#include "../controllers/PlayerController.h"

class PhysicSystem;

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
	// Where to updates the component systems. Responsibility can be moved to other places
	void updateComponentSystems(float dt);

private:
	struct Systems {
		PhysicSystem* physicSystem = nullptr;
	};

	Application* m_app;
	// Camera
	PerspectiveCamera m_cam;
	//FlyingCameraController m_camController;
	PlayerController m_playerController;

	const std::string createCube(const glm::vec3& position);

	Systems m_componentSystems;
	Scene m_scene;
	LightSetup m_lights;
	ConsoleCommands m_cc;
	std::unique_ptr<Model> m_cubeModel;
	std::unique_ptr<Model> m_planeModel;

};