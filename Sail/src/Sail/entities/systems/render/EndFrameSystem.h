#pragma once
#include "..//BaseComponentSystem.h"

class EndFrameSystem final : public BaseComponentSystem {
public:
	EndFrameSystem();
	~EndFrameSystem();

	void endAndPresentFrame();
};