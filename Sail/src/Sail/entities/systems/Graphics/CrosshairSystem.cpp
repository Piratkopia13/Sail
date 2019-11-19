#include "pch.h"
#include "CrosshairSystem.h"
#include "../../ECS.h"
#include "Sail/entities/components/Components.h"


CrosshairSystem::CrosshairSystem() {
	registerComponent<CrosshairComponent>(true, true, true);

	m_settings = &Application::getInstance()->getSettings();
}

CrosshairSystem::~CrosshairSystem() {

}

void CrosshairSystem::update(float dt) {
	for (auto& e : entities) {
		CrosshairComponent* c = e->getComponent<CrosshairComponent>();

		// get settings from settingStorage
		applySettings(c);

		// Update the crosshair if 'i' hit someone
		if (c->currentlyAltered) {
			alterCrosshair(e, dt);
		}
	}
}

void CrosshairSystem::applySettings(CrosshairComponent* c) {
	auto& dynamic = m_settings->applicationSettingsDynamic;

	c->thickness = dynamic["Crosshair"]["Thickness"].value;
	c->centerPadding = dynamic["Crosshair"]["CenterPadding"].value;
	c->size = dynamic["Crosshair"]["Size"].value;
	c->color.x = dynamic["Crosshair"]["Color R"].value;
	c->color.y = dynamic["Crosshair"]["Color G"].value;
	c->color.z = dynamic["Crosshair"]["Color B"].value;
	c->color.w = dynamic["Crosshair"]["Color A"].value;
}

void CrosshairSystem::alterCrosshair(Entity* e, float dt) {
	// Updates the crosshair based on how much time has passed
	// ---------------- 
	CrosshairComponent* c = e->getComponent<CrosshairComponent>();
	c->passedTimeSinceAlteration += dt;

	// If we're finished with the altered crosshair and want to go back to normal
	if (c->passedTimeSinceAlteration >= c->durationOfAlteredCrosshair) {
		c->currentlyAltered = false;
		c->passedTimeSinceAlteration = 0;
	}
	// Crosshair is updated in 'InGameGui.cpp'
}
