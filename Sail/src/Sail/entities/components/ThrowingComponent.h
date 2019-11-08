#pragma once
#include "Component.h"

#include <glm/vec3.hpp>


// TODO: Replace with game settings
constexpr auto MAX_CHARGE_TIME = 5.f;
constexpr auto MAX_THROW_CHARGE_MULT = 5.f;

class ThrowingComponent : public Component<ThrowingComponent> {
public:
	ThrowingComponent() {}
	~ThrowingComponent() {}

public:
	bool wasChargingLastFrame = false;
	bool isCharging = false;
	float chargeTime = 0.f;
	float maxChargingTime = MAX_CHARGE_TIME;
	float throwChargeMultiplier = 2.f;
	glm::vec3 direction = glm::vec3(0.f, 1.f, 0.f);

public:
#ifdef DEVELOPMENT
	void imguiRender(Entity** selected) {
		if (selected) {
			ThrowingComponent* throwingC = this;
			ImGui::Checkbox("wasChargingLastFrame", &wasChargingLastFrame);
			ImGui::Checkbox("isCharging", &isCharging);
			ImGui::SliderFloat("maxChargingTIme", &throwingC->maxChargingTime, 0.f, MAX_CHARGE_TIME);
			ImGui::SliderFloat("chargeTime", &throwingC->chargeTime, 0.f, throwingC->maxChargingTime);
			ImGui::SliderFloat("throwChargeMultiplier", &throwingC->throwChargeMultiplier, 0.f, MAX_THROW_CHARGE_MULT);
			ImGui::SliderFloat3("direction", &throwingC->direction.x, 0.f, 1.f);
		}
	}
#endif
};
