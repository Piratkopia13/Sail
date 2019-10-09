#pragma once

#include "Component.h"

class SpectatorComponent : public Component<SpectatorComponent> {
public:
	SpectatorComponent() {}
	~SpectatorComponent() {}
};