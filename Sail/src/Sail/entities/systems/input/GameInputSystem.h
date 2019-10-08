#pragma once
#include "../BaseComponentSystem.h"
#include "Sail/entities/Entity.h"
class Camera;
class CameraController;
class GameDataTracker;

struct Movement {
	float speedModifier = 1.f;
	float forwardMovement = 0.0f;
	float rightMovement = 0.0f;
	float upMovement = 0.0f;
};

class GameInputSystem final : public BaseComponentSystem {
public:
	GameInputSystem();
	~GameInputSystem();

	// This function is only here because it has to. use update with alpha.
	void update(float dt) { update(dt, 1.0f); }
	void update(float dt, float alpha);
	void initialize(Camera* cam);
	void clean();
	void updateCameraPosition(float alpha);

private:
	void processKeyboardInput(const float& dt);
	void processMouseInput(const float& dt);

	void putDownCandle(Entity* entity);
	Movement getPlayerMovementInput(Entity* entity);
	
	CameraController* m_cam = nullptr;
	GameDataTracker* m_gameDataTracker = nullptr;


	// --------- Earlier used variables below this line ---------
	float m_runSpeed = 2.0;
	float m_movementSpeed = 20.f;
	float m_runSoundTimer = 0.0f;
	bool m_wasSpacePressed = false;
	float m_projectileSpawnCounter = 0.f;
	float m_lookSensitivityMouse = 0.1;

	// #netcodeNote not thread safe, might cause issues
	float m_yaw, m_pitch, m_roll;

};