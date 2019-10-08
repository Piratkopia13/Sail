#pragma once

#include "Component.h"
#include "../Entity.h"
#include "Sail/ai/pathfinding/NodeSystem.h"
#include <glm/glm.hpp>

class SpectatorComponent : public Component<SpectatorComponent> {
public:
	SpectatorComponent();
	~SpectatorComponent();
};