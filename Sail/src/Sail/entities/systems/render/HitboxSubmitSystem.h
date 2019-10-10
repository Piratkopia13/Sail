#pragma once
#include "..//BaseComponentSystem.h"

class HitboxSubmitSystem final : public BaseComponentSystem {
public:
	HitboxSubmitSystem();
	~HitboxSubmitSystem();

	void toggleHitboxes();
	void submitAll();

private:
	bool m_renderHitBoxes;
};
