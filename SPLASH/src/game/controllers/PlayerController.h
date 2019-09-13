#pragma once

class CameraController;
class Camera;
class Entity;

class PlayerController {
public:
	PlayerController(Camera* cam);
	~PlayerController();

	void update(float dt);

	void setStartPosition(const glm::vec3& pos);

	std::shared_ptr<Entity> getEntity();

private:
	float m_movementSpeed = 5.f;

	// "Attached" camera
	CameraController* m_cam;

	std::shared_ptr<Entity> m_player;

	float m_yaw, m_pitch, m_roll;

	float m_lookSensitivityMouse = 0.1f;

};