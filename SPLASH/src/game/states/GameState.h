#pragma once
#include "Sail.h"
#include "Sail/entities/systems/SystemDeclarations.h"

class NetworkSerializedPackageEvent;

class GameState : public State {
public:
	GameState(StateStack& stack);
	~GameState();

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
	bool onNetworkSerializedPackageEvent(NetworkSerializedPackageEvent& event);

	bool onPlayerCandleDeath(PlayerCandleDeathEvent& event);
	bool renderImguiConsole(float dt);
	bool renderImguiProfiler(float dt);
	bool renderImGuiRenderSettings(float dt);
	bool renderImGuiLightDebug(float dt);

	void shutDownGameState();

	// Where to updates the component systems. Responsibility can be moved to other places
	void updatePerTickComponentSystems(float dt);
	void updatePerFrameComponentSystems(float dt, float alpha);
	void runSystem(float dt, BaseComponentSystem* toRun);

	Entity::SPtr createCandleEntity(const std::string& name, Model* lightModel, Model* bbModel, glm::vec3 lightPos);

	void loadAnimations();
	void initAnimations();

private:
	Application* m_app;
	// Camera
	PerspectiveCamera m_cam;

	// TODO: Only used for AI, should be removed once AI can target player in a better way.
	Entity* m_player;

	void createTestLevel(Shader* shader, Model* boundingBoxModel);
	void setUpPlayer(Model* boundingBoxModel, Model* projectileModel, Model* lightModel, unsigned char playerID);
	void createBots(Model* boundingBoxModel, Model* characterModel, Model* projectileModel, Model* lightModel);
	void createLevel(Shader* shader, Model* boundingBoxModel);
	const std::string createCube(const glm::vec3& position);

	Systems m_componentSystems;
	LightSetup m_lights;
	ConsoleCommands m_cc;
	Profiler m_profiler;

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