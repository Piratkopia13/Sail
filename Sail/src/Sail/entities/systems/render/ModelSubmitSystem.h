#pragma once
#include "..//BaseComponentSystem.h"

class ModelSubmitSystem final : public BaseComponentSystem {
public:
	ModelSubmitSystem();
	~ModelSubmitSystem();

	void submitAll(const float alpha);
};