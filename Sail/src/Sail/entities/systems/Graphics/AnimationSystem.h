#pragma once
#include "..//BaseComponentSystem.h"

class AnimationSystem final : public BaseComponentSystem {
public:
	AnimationSystem();
	~AnimationSystem();

	void update(float dt) override;

private:
};