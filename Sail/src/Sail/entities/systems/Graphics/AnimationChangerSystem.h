#pragma once

#include "../BaseComponentSystem.h"

class AnimationChangerSystem final : public BaseComponentSystem {
public:
	AnimationChangerSystem();
	~AnimationChangerSystem();

	void update(float dt);
};