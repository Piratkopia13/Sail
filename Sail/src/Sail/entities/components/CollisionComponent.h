#pragma once
#include "Component.h"
#include "..//..//Physics/Octree.h"

class CollisionComponent : public Component<CollisionComponent> {
public:
	CollisionComponent(bool simpleCollisions = false);
	~CollisionComponent();

	float drag;
	float bounciness;
	float padding;
	bool onGround;
	bool doSimpleCollisions;

	std::vector<Octree::CollisionInfo> collisions; //Contains the info for current collisions
};