#pragma once

#include "Sail.h"
#include "../controllers/PlayerController.h"

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
	bool renderImguiProfiler(float dt);

private:
	Application* m_app;
	// Camera
	PerspectiveCamera m_cam;
	//FlyingCameraController m_camController;
	PlayerController m_playerController;

	//SimplePhysicsSystem m_physSystem;

	Entity::SPtr m_texturedCubeEntity;
	std::vector<Entity::SPtr> m_transformTestEntities;

	const std::string createCube(const glm::vec3& position);

	Scene m_scene;
	LightSetup m_lights;
	ConsoleCommands m_cc;
	Profiler m_profiler;
	// ImGUI profiler data
	float m_profilerTimer = 0.f;
	int m_profilerCounter = 0;
	float* m_virtRAMHistory;
	float* m_physRAMHistory;
	float* m_cpuHistory;
	float* m_vramUsageHistory;
	float* m_frameTimesHistory;
	std::string m_virtCount;
	std::string m_physCount;
	std::string m_vramUCount;
	std::string m_cpuCount;
	std::string m_ftCount;

	// Uncomment this to enable vram budget visualization
	//std::string m_vramBCount;
	//float* m_vramBudgetHistory;

	std::unique_ptr<Model> m_cubeModel;
	std::unique_ptr<Model> m_planeModel;

};