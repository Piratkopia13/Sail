#pragma once
#include "../BaseComponentSystem.h"

class AnimationInitSystem final : public BaseComponentSystem {
public:
	AnimationInitSystem();
	~AnimationInitSystem();

	void loadAnimations();
	void initAnimations();
};