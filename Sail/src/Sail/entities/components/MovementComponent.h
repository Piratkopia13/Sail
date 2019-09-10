#pragma once

#include "Component.h"
#include <glm/glm.hpp>

class MovementComponent : public Component {
public:
	SAIL_COMPONENT MovementComponent(const float& initialSpeed, const glm::vec3& initialDirection) {
		m_speed = initialSpeed;
		m_direction = initialDirection;
	}
	~MovementComponent() {}

	void setSpeed(const float& speed);
	void addSpeed(const float& speed);
	void setDirection(const glm::vec3& direction);

	const float& getSpeed() const;
	const glm::vec3& getDirection() const;

private:
	glm::vec3 m_direction;
	float m_speed;

};