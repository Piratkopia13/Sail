#pragma once
#include "Component.h"
#include <glm/vec3.hpp>

class MovementComponent final : public Component<MovementComponent> {
public:
	MovementComponent() {}
	~MovementComponent() {}

	glm::vec3 velocity = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 constantAcceleration = glm::vec3(0.0f);
	glm::vec3 accelerationToAdd = glm::vec3(0.0f);

	glm::vec3 oldVelocity = glm::vec3(0.0f);
	glm::vec3 oldMovement = glm::vec3(0.0f);

	float airDrag = 1.0f;

	float updateableDt = 0.0f;
};