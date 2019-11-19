#pragma once

#include "Component.h"
#include "..//..//Physics/Octree.h"

#include "Sail/graphics/geometry/Transform.h"

class Model;

class RagdollComponent : public Component<RagdollComponent> {
public:
	RagdollComponent();
	RagdollComponent(Model* wireframe);
	~RagdollComponent();

	void addContactPoint(glm::vec3 localOffset, glm::vec3 halfSize);

public:
	struct ContactPoint {
		BoundingBox boundingBox;
		glm::vec3 localOffSet;
		std::vector<Octree::CollisionInfo> collisions;
		Transform transform;
	};

	std::vector<ContactPoint> contactPoints;

	glm::vec3 localCenterOfMass;
	Model* wireframeModel;
};
