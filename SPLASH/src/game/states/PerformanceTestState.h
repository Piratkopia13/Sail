#pragma once

#include "Sail.h"

class AiSystem;
class AnimationSystem;
class CandleSystem;
class EntityAdderSystem;
class EntityRemovalSystem;
class LifeTimeSystem;
class LightSystem;
class OctreeAddRemoverSystem;
class MovementSystem;
class MovementPostCollisionSystem;
class CollisionSystem;
class SpeedLimitSystem;
class PrepareUpdateSystem;
class GunSystem;
class ProjectileSystem;
class LevelGeneratorSystem;
class AudioSystem;
class RenderSystem;

class PerformanceTestState : public State {
public:
	PerformanceTestState(StateStack& stack);
	~PerformanceTestState();

	// Process input for the state ||
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
	// If the state is about to change clean it up
	virtual bool prepareStateChange() override;


private:
	bool onResize(WindowResizeEvent& event);
	bool renderImguiProfiler(float dt);
	bool renderImGuiRenderSettings(float dt);
	bool renderImGuiLightDebug(float dt);
	bool renderImGuiGameValues(float dt);

	void shutDownPerformanceTestState();

	// Where to updates the component systems. Responsibility can be moved to other places
	void updatePerTickComponentSystems(float dt);
	void updatePerFrameComponentSystems(float dt, float alpha);
	void runSystem(float dt, BaseComponentSystem* toRun);

	Entity::SPtr createCandleEntity(const std::string& name, Model* lightModel, Model* bbModel, glm::vec3 lightPos);

	void loadAnimations();
	void initAnimations();

	void populateScene(Model* characterModel, Model* lightModel, Model* bbModel, Model* projectileModel, Shader* shader);

private:
	struct Systems {
		AiSystem* aiSystem = nullptr;
		AnimationSystem* animationSystem = nullptr;
		CandleSystem* candleSystem = nullptr;
		EntityAdderSystem* entityAdderSystem = nullptr;
		EntityRemovalSystem* entityRemovalSystem = nullptr;
		LifeTimeSystem* lifeTimeSystem = nullptr;
		LightSystem* lightSystem = nullptr;
		OctreeAddRemoverSystem* octreeAddRemoverSystem = nullptr;
		UpdateBoundingBoxSystem* updateBoundingBoxSystem = nullptr;
		PrepareUpdateSystem* prepareUpdateSystem = nullptr;
		GunSystem* gunSystem = nullptr;
		ProjectileSystem* projectileSystem = nullptr;
		AudioSystem* audioSystem = nullptr;
		RenderSystem* renderSystem = nullptr;
		LevelGeneratorSystem* levelGeneratorSystem = nullptr;
		MovementSystem* movementSystem = nullptr;
		MovementPostCollisionSystem* movementPostCollisionSystem = nullptr;
		CollisionSystem* collisionSystem = nullptr;
		SpeedLimitSystem* speedLimitSystem = nullptr;
	};

	Application* m_app;
	// Camera
	PerspectiveCamera m_cam;
	FlyingCameraController m_camController;

	void createBots(Model* boundingBoxModel, Model* characterModel, Model* projectileModel, Model* lightModel);
	void createLevel(Shader* shader, Model* boundingBoxModel);

	Systems m_componentSystems;
	LightSetup m_lights;
	Profiler m_profiler;

	std::vector<Entity::SPtr> m_performanceEntities;

	size_t m_currLightIndex;

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

	bool m_paused = false;
	bool m_isSingleplayer = true;

	Octree* m_octree;
	bool m_disableLightComponents;
	bool m_showcaseProcGen;

	std::bitset<MAX_NUM_COMPONENTS_TYPES> m_currentlyWritingMask;
	std::bitset<MAX_NUM_COMPONENTS_TYPES> m_currentlyReadingMask;

	std::vector<std::future<BaseComponentSystem*>> m_runningSystemJobs;
	std::vector<BaseComponentSystem*> m_runningSystems;

	bool m_poppedThisFrame = false;

};