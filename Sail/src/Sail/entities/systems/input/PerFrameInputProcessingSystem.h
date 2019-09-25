#pragma once
#include "..//BaseComponentSystem.h"

class PerFrameInputProcessingSystem final : public BaseComponentSystem {
public:
	PerFrameInputProcessingSystem();
	~PerFrameInputProcessingSystem();

	void update(float dt) override;

private:
};