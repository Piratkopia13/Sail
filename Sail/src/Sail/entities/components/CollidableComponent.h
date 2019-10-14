#pragma once
#include "Component.h"

// Name is confusing. This is simply a flag where only entities with this component are inserted into the octree
class CollidableComponent : public Component<CollidableComponent> {
public:
	CollidableComponent() { }
	~CollidableComponent() { }
};