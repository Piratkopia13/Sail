#pragma once
#include "..//BaseComponentSystem.h"

class MetaballSubmitSystem final : public BaseComponentSystem {
public:
	MetaballSubmitSystem();
	~MetaballSubmitSystem();

	void submitAll(const float alpha);
};