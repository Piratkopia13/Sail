#pragma once
#include "Component.h"
#include "..//Physics/Physics.h"
#include <glm/vec3.hpp>

class PhysicsComponent : public Component<PhysicsComponent> {
public:
	PhysicsComponent(const glm::vec3& _velocity = glm::vec3(0, 0, 0), const glm::vec3& _rotation = glm::vec3(0, 0, 0), const glm::vec3& _acceleration = glm::vec3(0, 0, 0))
		: velocity(_velocity), rotation(_rotation), acceleration(_acceleration) {
	}
	glm::vec3 velocity;
	glm::vec3 rotation;		// Should be quaternions instead of Euler angles in the future
	glm::vec3 acceleration;

	std::vector<Octree::CollisionInfo> collisions; //Contains the info for current collisions
	//glm::vec3 rotationAxis;
	//float angularVelocity;
};