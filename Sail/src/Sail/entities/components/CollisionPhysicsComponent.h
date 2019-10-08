#pragma once
#include "Component.h"
#include "..//..//Physics/Octree.h"

class CollisionComponent : public Component<CollisionComponent> {
public:
	CollisionComponent() {}
	~CollisionComponent() {}

	float airDrag = 1.0f;
	float drag = 25.0f;
	float bounciness = 0.0f;
	float padding = -1.0f;
	bool onGround = false;

	std::vector<Octree::CollisionInfo> collisions; //Contains the info for current collisions
};