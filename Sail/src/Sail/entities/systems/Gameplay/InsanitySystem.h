#pragma once
#include "..//BaseComponentSystem.h"

class InsanitySystem final : public BaseComponentSystem {
public:
	InsanitySystem();
	~InsanitySystem();

	void update(float dt) override;

	float m_maxInsanity = 100;
	float m_minInsanity = 0;
private:
};