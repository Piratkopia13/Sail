#pragma once
#include "..//BaseComponentSystem.h"

class EntityAdderSystem final : public BaseComponentSystem {
public:
	EntityAdderSystem();
	~EntityAdderSystem();

	void update(float dt) override;
};