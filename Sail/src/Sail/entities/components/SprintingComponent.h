#pragma once

#include "Component.h"

// TODO: Replace with game settings
constexpr auto MAX_SPRINT_TIME = 5.f;
constexpr auto MAX_SPRINT_DOWN_TIME = 5.f;
constexpr auto RECOVERY_TIME = 0.5f;

class SprintingComponent : public Component<SprintingComponent> {
public:
	SprintingComponent() {}
	~SprintingComponent() {}

#ifdef DEVELOPMENT
	void imguiRender(Entity** selected) {
		SprintingComponent* sprintC = this;
		ImGui::SliderFloat("speedModifier", &sprintC->sprintSpeedModifier, 0.f, 2.f);
		ImGui::SliderFloat("sprintTimer", &sprintC->sprintTimer, 0.f, MAX_SPRINT_TIME);
		ImGui::SliderFloat("downTimer", &sprintC->downTimer, 0.f, MAX_SPRINT_DOWN_TIME);
		ImGui::Checkbox("Exhausted", &sprintC->exhausted);
		ImGui::Checkbox("Can sprint", &sprintC->canSprint);
		ImGui::Checkbox("Do sprint", &sprintC->doSprint);
	}
#endif

public:
	float sprintSpeedModifier = 1.3f;
	float sprintTimer = 0.f;
	float downTimer = 0.f;
	
	bool sprintedLastFrame = false;
	bool doSprint = false;
	bool canSprint = false;
	bool exhausted = false;
};