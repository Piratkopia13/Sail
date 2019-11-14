#pragma once
#include "Component.h"

class SpeedLimitComponent final : public Component<SpeedLimitComponent> {
public:
	SpeedLimitComponent(float maxSpeed_ = INFINITY) 
		: maxSpeed(maxSpeed_)
		, normalMaxSpeed(maxSpeed) {}
	~SpeedLimitComponent() {}

	float maxSpeed;
	float normalMaxSpeed;
	void imguiRender(Entity** selected) {
		ImGui::Text("Speed"); ImGui::SameLine();
		if (ImGui::DragFloat("##aIndex", &maxSpeed, 0.1f)) {
		}
	}
};