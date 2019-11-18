#pragma once

#include "Component.h"
#include "..//..//Physics/Octree.h"

class RagdollComponent : public Component<RagdollComponent> {
public:
	RagdollComponent();
	~RagdollComponent();

	struct ContactPoints {
		BoundingBox boundingBox;
		glm::vec3 localOffSet;
		Octree::CollisionInfo collisions;
	};

	std::vector<ContactPoints> contactPoints;

	glm::vec3 localCenterOfMass;
};
