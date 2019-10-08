#pragma once
#include "Component.h"
#include <glm/vec3.hpp>

class MovementComponent final : public Component<MovementComponent> {
public:
	MovementComponent() {}
	~MovementComponent() {}

	glm::vec3 velocity = glm::vec3(0.0f);
	glm::vec3 rotation = glm::vec3(0.0f);
	glm::vec3 acceleration = glm::vec3(0.0f);
	glm::vec3 accelerationToAdd = glm::vec3(0.0f);

	glm::vec3 oldVelocity = glm::vec3(0.0f);
};