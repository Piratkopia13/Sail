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
class GameInputSystem;
class NetworkReceiverSystem;
class NetworkSenderSystem;
class AudioSystem;
class RenderSystem;
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
	bool renderImGuiLightDebug(float dt);

	void shutDownGameState();

	// Where to updates the component systems. Responsibility can be moved to other places
	void updatePerTickComponentSystems(float dt);
	void updatePerFrameComponentSystems(float dt, float alpha);
	void runSystem(float dt, BaseComponentSystem* toRun);

	void loadAnimations();
	void initAnimations();

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
		GameInputSystem* gameInputSystem = nullptr;
		NetworkReceiverSystem* networkReceiverSystem = nullptr;
		NetworkSenderSystem* networkSenderSystem = nullptr;
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

	// TODO: Only used for AI, should be removed once AI can target player in a better way.
	Entity* m_player;

	void createTestLevel(Shader* shader, Model* boundingBoxModel);
	void createBots(Model* boundingBoxModel, Model* characterModel, Model* projectileModel, Model* lightModel);
	void createLevel(Shader* shader, Model* boundingBoxModel);
	const std::string createCube(const glm::vec3& position);
	const std::string teleportToMap();

	Systems m_componentSystems;
	LightSetup m_lights;
	Profiler m_profiler;
	RenderSettingsWindow m_renderSettingsWindow;

	size_t m_currLightIndex;

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