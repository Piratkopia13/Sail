#pragma once
#include "Component.h"

class SpeedLimitComponent final : public Component<SpeedLimitComponent> {
public:
	SpeedLimitComponent(float maxSpeed_ = INFINITY) : maxSpeed(maxSpeed_){}
	~SpeedLimitComponent() {}

	float maxSpeed;
};