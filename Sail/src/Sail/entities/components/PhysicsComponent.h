#pragma once
#include "Component.h"
#include <glm/vec3.hpp>

class PhysicsComponent : public Component<PhysicsComponent> {
public:
	PhysicsComponent(const glm::vec3& _velocity = glm::vec3(0, 0, 0), const glm::vec3& _rotation = glm::vec3(0, 0, 0), const glm::vec3& _acceleration = glm::vec3(0, 0, 0)) :
		velocity(_velocity), rotation(_rotation), acceleration(_acceleration) {

	}
	glm::vec3 velocity;
	glm::vec3 acceleration;
	glm::vec3 rotation;		// Should not be euler angles in the future
	//glm::vec3 rotationAxis;
	//float angularVelocity;
};