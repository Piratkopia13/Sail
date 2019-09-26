#pragma once
#include "../BaseComponentSystem.h"

class PhysicsComponent;
class CameraController;
class PlayerController;
class Entity;

class GameInputSystem final : public BaseComponentSystem {
public:
	GameInputSystem();
	~GameInputSystem();

	// This function is only here because it has to. use update with alpha.
	void update(float dt) {}
	void update(float dt, float alpha);
	void initialize(PlayerController* playerEntity);
	void updateCameraPosition(float alpha);

	void processPerFrameInput();
	void processPerTickInput();

private:
	void processKeyboardInput(const float& dt);
	void processMouseInput(const float& dt);

	PlayerController* m_player = nullptr;
	Entity* m_playerEntity = nullptr;
	PhysicsComponent* m_physics = nullptr;
	CameraController* m_cam = nullptr;

	// --------- Earlier used variables below this line ---------
	float m_runSpeed = 2.0;
	float m_movementSpeed = 20.f;
	bool m_wasSpacePressed = false;
	float m_projectileSpawnCounter = 0.f;
	float m_lookSensitivityMouse = 0.1;

	// #netcodeNote not thread safe, might cause issues
	float m_yaw, m_pitch, m_roll;

	glm::vec3 calculateNormalizedRightVector();



};