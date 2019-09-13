#pragma once

class Entity;

class AiController {
public:
	AiController();
	AiController(std::shared_ptr<Entity> toControl);
	~AiController();

	void update();

	void setEntity(std::shared_ptr<Entity> toControl);
	std::shared_ptr<Entity> getEntity();
	

private:
	float m_movementSpeed = 5.f;

	std::shared_ptr<Entity> m_controlledEntity;

	float m_yaw, m_pitch, m_roll;

	float m_lookSensitivityMouse = 0.1f;

};