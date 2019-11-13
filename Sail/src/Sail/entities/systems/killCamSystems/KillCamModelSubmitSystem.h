#pragma once
#include "..//BaseComponentSystem.h"

class KillCamModelSubmitSystem final : public BaseComponentSystem {
public:
	KillCamModelSubmitSystem();
	~KillCamModelSubmitSystem();

	void submitAll(const float alpha);
};