#pragma once
#include "Component.h"
#include <glm/vec3.hpp>

class PhysicsComponent : public Component {
public:
	SAIL_COMPONENT

	PhysicsComponent(const glm::vec3& _velocity = glm::vec3(0, 0, 0), const glm::vec3& _rotation = glm::vec3(0, 0, 0)) :
		velocity(_velocity), rotation(_rotation) {

	}
	glm::vec3 velocity;
	glm::vec3 rotation;		// Should not be euler angles
	//glm::vec3 rotationAxis;
	//float angularVelocity;
};