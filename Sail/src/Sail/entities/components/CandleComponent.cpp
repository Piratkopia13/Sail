#include "pch.h"
#include "CandleComponent.h"

void CandleComponent::imguiRender(Entity** e) {
	ImGui::InputInt("Lives", &respawns);
}
