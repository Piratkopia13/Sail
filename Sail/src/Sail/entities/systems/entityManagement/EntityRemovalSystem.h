#pragma once
#include "..//BaseComponentSystem.h"

class EntityRemovalSystem final : public BaseComponentSystem {
public:
	EntityRemovalSystem();
	~EntityRemovalSystem();
	void update(float dt) override;
};