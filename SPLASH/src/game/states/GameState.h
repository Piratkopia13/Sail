#pragma once

#include "Sail.h"

class AiSystem;
class AnimationSystem;
class CandleSystem;
class EntityRemovalSystem;
class LifeTimeSystem;
class LightSystem;
class OctreeAddRemoverSystem;
class PhysicSystem;
class PrepareUpdateSystem;
class GunSystem;
class ProjectileSystem;
class GameInputSystem;
class AudioSystem;
class RenderSystem;

class GameState : public State {
public:
	GameState(StateStack& stack);
	~GameState();

	// Process input for the state
	virtual bool processInput(float dt) override;
	// Sends events to the state
	virtual bool onEvent(Event& event) override;
	// Updates the state - Runs every frame
	virtual bool update(float dt, float alpha = 1.0f) override;
	// Updates the state - Runs every tick
	virtual bool fixedUpdate(float dt) override;
	// Renders the state
	virtual bool render(float dt, float alpha = 1.0f) override;
	// Renders imgui
	virtual bool renderImgui(float dt) override;



private:
	bool onResize(WindowResizeEvent& event);
	bool renderImguiConsole(float dt);
	bool renderImguiProfiler(float dt);
	bool renderImGuiRenderSettings(float dt);
	bool renderImGuiLightDebug(float dt);

	// Where to updates the component systems. Responsibility can be moved to other places
	void updatePerTickComponentSystems(float dt);
	void updatePerFrameComponentSystems(float dt, float alpha);
	void runSystem(float dt, BaseComponentSystem* toRun);

	Entity::SPtr createCandleEntity(const std::string& name, Model* lightModel, glm::vec3 lightPos);

private:
	struct Systems {
		AiSystem* aiSystem = nullptr;
		AnimationSystem* animationSystem = nullptr;
		CandleSystem* candleSystem = nullptr;
		EntityRemovalSystem* entityRemovalSystem = nullptr;
		LifeTimeSystem* lifeTimeSystem = nullptr;
		LightSystem* lightSystem = nullptr;
		OctreeAddRemoverSystem* octreeAddRemoverSystem = nullptr;
		PhysicSystem* physicSystem = nullptr;
		UpdateBoundingBoxSystem* updateBoundingBoxSystem = nullptr;
		PrepareUpdateSystem* prepareUpdateSystem = nullptr;
		GunSystem* gunSystem = nullptr;
		ProjectileSystem* projectileSystem = nullptr;
		GameInputSystem* gameInputSystem = nullptr;
		AudioSystem* audioSystem = nullptr;
		RenderSystem* renderSystem = nullptr;
	};

	Application* m_app;
	// Camera
	PerspectiveCamera m_cam;

	// TODO: Only used for AI, should be removed once AI can target player in a better way.
	Entity* m_player;

	const std::string createCube(const glm::vec3& position);

	Systems m_componentSystems;
	LightSetup m_lights;
	ConsoleCommands m_cc;
	Profiler m_profiler;

	size_t m_currLightIndex;
	// For use by non-deterministic entities
	const float* pAlpha = nullptr;

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


	std::unique_ptr<Model> m_cubeModel;
	std::unique_ptr<Model> m_planeModel;
	

	std::unique_ptr<Model> m_boundingBoxModel;

	Octree* m_octree;
	bool m_disableLightComponents;

	std::bitset<MAX_NUM_COMPONENTS_TYPES> m_currentlyWritingMask;
	std::bitset<MAX_NUM_COMPONENTS_TYPES> m_currentlyReadingMask;

	std::vector<std::future<BaseComponentSystem*>> m_runningSystemJobs;
	std::vector<BaseComponentSystem*> m_runningSystems;
};