#pragma once
#include "..//BaseComponentSystem.h"

class ProjectileSystem final : public BaseComponentSystem {
public:
	ProjectileSystem();
	~ProjectileSystem();

	void update(float dt) override;
};
