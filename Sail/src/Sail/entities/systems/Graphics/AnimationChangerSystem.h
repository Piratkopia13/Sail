#pragma once

#include "../BaseComponentSystem.h"

class AnimationChangerSystem final : public BaseComponentSystem {
public:
	AnimationChangerSystem();
	~AnimationChangerSystem();

	void update(float dt);

	enum MovementIndex {
	DEFAULT,
	IDLE,
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	IDLEJUMP,
	FORWARDJUMP,
	DROP,
	//THROW
	};
};