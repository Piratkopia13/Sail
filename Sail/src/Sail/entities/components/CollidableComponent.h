#pragma once
#include "Component.h"

class CollidableComponent : public Component<CollidableComponent> {
public:
	CollidableComponent() { }
	~CollidableComponent() { }
};