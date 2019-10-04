#pragma once
#include "Component.h"
#include "..//Physics/Sphere.h"

class CollisionSpheresComponent : public Component<CollisionSpheresComponent> {
public:
	CollisionSpheresComponent() {}
	~CollisionSpheresComponent() {}

	Sphere spheres[2];
	//Sphere sphere;
};