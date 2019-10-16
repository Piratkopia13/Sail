#pragma once
#include "../../BaseComponentSystem.h"

class RenderImGuiSystem final : public BaseComponentSystem {
public:
	RenderImGuiSystem();
	~RenderImGuiSystem();

	void renderImGuiAnimationSettings();
};