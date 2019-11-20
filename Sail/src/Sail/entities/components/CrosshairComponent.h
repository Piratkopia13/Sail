#pragma once


#include "Component.h"

class CrosshairComponent : public Component<CrosshairComponent> {
public:
	CrosshairComponent() {
	}
	~CrosshairComponent() {}


#ifdef DEVELOPMENT
	void imguiRender(Entity** relevantEntity) {

	}
#endif

	// Hit Detection and alteration
	bool currentlyAltered = false;
	float passedTimeSinceAlteration = 0.0f;
	float durationOfAlteredCrosshair = 0.5f;

	// Turn off while
	bool sprinting = false;

	// Rendering settings
	ImVec4 color = ImVec4(1.0f, 1.0f, 0.4f, 1.0f);
	float thickness = 1.0f;
	float centerPadding = 10.0f;
	float size = 200.0f;
};