#pragma once
#include "Component.h"
#include "..//..//Physics/Octree.h"

class CollisionComponent : public Component<CollisionComponent> {
public:
	CollisionComponent();
	~CollisionComponent();

	float drag;
	float bounciness;
	float padding;
	bool onGround;

	std::vector<Octree::CollisionInfo> collisions; //Contains the info for current collisions
};