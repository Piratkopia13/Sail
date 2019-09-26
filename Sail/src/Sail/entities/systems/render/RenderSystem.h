#pragma once
#include "..//BaseComponentSystem.h"

class RenderSystem : public BaseComponentSystem {
public:
	RenderSystem();
	~RenderSystem();
	void update(float dt) override;
};