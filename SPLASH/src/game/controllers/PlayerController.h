#pragma once

class CameraController;
class Camera;
class Entity;

constexpr float RUN_SPEED = 2.0f;

class PlayerController {
public:
	PlayerController(Camera* cam);
	~PlayerController();

	void update(float dt);

	void setStartPosition(const glm::vec3& pos);
	void prepareUpdate();

	void processKeyboardInput(float dt);
	void processMouseInput(float dt);

	std::shared_ptr<Entity> getEntity();

private:
	float m_movementSpeed = 20.f;

	// "Attached" camera
	CameraController* m_cam;

	std::shared_ptr<Entity> m_player;

	// #netcodeNote not thread safe, might cause issues
	float m_yaw, m_pitch, m_roll;

	float m_lookSensitivityMouse = 0.1f;

};