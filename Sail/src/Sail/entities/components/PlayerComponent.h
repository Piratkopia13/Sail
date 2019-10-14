#pragma once
#include "Component.h"

class PlayerComponent : public Component<PlayerComponent> {
public:
	PlayerComponent() { }
	~PlayerComponent() { }
};