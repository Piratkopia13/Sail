#pragma once
#include "..//BaseComponentSystem.h"

class BoundingboxSubmitSystem final : public BaseComponentSystem {
public:
	BoundingboxSubmitSystem();
	~BoundingboxSubmitSystem();

	void toggleHitboxes();
	void submitAll();

private:
	bool m_renderHitBoxes;
};
