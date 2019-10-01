#pragma once
#include "..//BaseComponentSystem.h"

class GunSystem final : public BaseComponentSystem {
public:
	GunSystem();
	~GunSystem();

	void update(float dt);
};
