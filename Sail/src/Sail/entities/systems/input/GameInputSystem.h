#pragma once
#include "../BaseComponentSystem.h"
#include "Sail/entities/Entity.h"
class Camera;
class CameraController;
class GameDataTracker;


class GameInputSystem final : public BaseComponentSystem {
public:
	GameInputSystem();
	~GameInputSystem();

	// This function is only here because it has to. use update with alpha.
	void update(float dt) { update(dt, 1.0f); }
	void update(float dt, float alpha);
	void initialize(Camera* cam);
	void updateCameraPosition(float alpha);

	void processPerFrameInput();
	void processPerTickInput();

private:
	void processKeyboardInput(const float& dt);
	void processMouseInput(const float& dt);

	CameraController* m_cam = nullptr;
	GameDataTracker* m_gameDataTracker = nullptr;


	// --------- Earlier used variables below this line ---------
	bool m_songStarted = false;
	float m_runSpeed = 2.0;
	float m_movementSpeed = 20.f;
	float m_runSoundTimer = 0.0f;
	float m_fallTimer = 0.0f;
	bool m_wasSpacePressed = false;
	bool m_hasLanded = true;
	float m_projectileSpawnCounter = 0.f;
	float m_lookSensitivityMouse = 0.1;

	// #netcodeNote not thread safe, might cause issues
	float m_yaw, m_pitch, m_roll;

	glm::vec3 calculateNormalizedRightVector();



};