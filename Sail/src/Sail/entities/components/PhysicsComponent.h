#pragma once
#include "Component.h"
#include "..//Physics/Physics.h"
#include <glm/vec3.hpp>

class PhysicsComponent : public Component<PhysicsComponent> {
public:
	PhysicsComponent(const glm::vec3& _velocity = glm::vec3(0, 0, 0), const glm::vec3& _rotation = glm::vec3(0, 0, 0), const glm::vec3& _acceleration = glm::vec3(0, 0, 0))
		: velocity(_velocity), constantRotation(_rotation), constantAcceleration(_acceleration) {
	}
	glm::vec3 velocity;
	glm::vec3 constantRotation;		// Should probably be quaternions instead of Euler angles in the future
	glm::vec3 constantAcceleration;

	glm::vec3 accelerationToAdd = glm::vec3(0.0f);

	float maxSpeed = INFINITY;

	float airDrag = 1.0f;
	float drag = 25.0f;

	float bounciness = 0.0f;

	bool onGround = false;

	std::vector<Octree::CollisionInfo> collisions; //Contains the info for current collisions

private:
	friend class PhysicSystem;
	glm::vec3 m_oldVelocity = glm::vec3(0.0f);
};