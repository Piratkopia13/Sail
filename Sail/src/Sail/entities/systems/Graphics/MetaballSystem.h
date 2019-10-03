#pragma once
#include "../BaseComponentSystem.h"

class MetaballSystem : public BaseComponentSystem {
	MetaballSystem();
	~MetaballSystem();

	virtual void update(float dt);
};

