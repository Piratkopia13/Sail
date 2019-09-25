#pragma once
#include "..//BaseComponentSystem.h"
#include "Sail.h"

class ProjectileSystem final : public BaseComponentSystem {
public:
	ProjectileSystem();
	~ProjectileSystem();

	void update(float dt, Scene* scene);
	void update(float dt) override;
};
