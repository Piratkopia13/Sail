#pragma once


#include "Component.h"


class CrosshairComponent : public Component<CrosshairComponent> {
public:
	CrosshairComponent() {}
	~CrosshairComponent() {}


#ifdef DEVELOPMENT
	void imguiRender(Entity** relevantEntity) {

	}
#endif

	// Normal color
	// Altered color

	bool recentlyHitSomeone = false;
	float passedTimeSinceAlteration = 0.0f;
	float durationOfAlteredCrosshair = 0.5f;
};