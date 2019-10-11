#pragma once
#include "..//BaseComponentSystem.h"

class RealTimeModelSubmitSystem final : public BaseComponentSystem {
public:
	RealTimeModelSubmitSystem();
	~RealTimeModelSubmitSystem();

	void submitAll(const float alpha);
};