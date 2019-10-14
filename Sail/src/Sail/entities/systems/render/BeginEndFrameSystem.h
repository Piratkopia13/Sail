#pragma once
#include "..//BaseComponentSystem.h"
#include "..//..//..//graphics/camera/Camera.h"

class BeginEndFrameSystem final : public BaseComponentSystem {
public:
	BeginEndFrameSystem();
	~BeginEndFrameSystem();

	void renderNothing();

	void beginFrame(Camera& camera);
	void endFrameAndPresent();
};