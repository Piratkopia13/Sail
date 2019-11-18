#include "pch.h"
#include "CrosshairSystem.h"
#include "../../ECS.h"
#include "Sail/entities/components/Components.h"


CrosshairSystem::CrosshairSystem() {
	registerComponent<CrosshairComponent>(true, true, true);
}

CrosshairSystem::~CrosshairSystem() {


}

void CrosshairSystem::update(float dt) {
	// render the crosshair


	for (auto& e : entities) {
		CrosshairComponent* c = e->getComponent<CrosshairComponent>();

		if (c->recentlyHitSomeone) {

			alterCrosshair(e, dt);
		}
	}
}

void CrosshairSystem::alterCrosshair(Entity* e, float dt) {
	// Updates the crosshair based on how much time has passed
	// ---------------- 
	CrosshairComponent* c = e->getComponent<CrosshairComponent>();
	c->passedTimeSinceAlteration += dt;

	// If we're finished with the altered crosshair and want to go back to normal
	if (c->passedTimeSinceAlteration >= c->durationOfAlteredCrosshair) {
		c->recentlyHitSomeone = false;
		c->passedTimeSinceAlteration = 0;
		// Set color to 'normal'
	}
	// If we're not finished with the altered crosshair yet.
	else {
		// Set color to 'altered'
	}
}
