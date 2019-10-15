#include "pch.h"
#include "LightSystem.h"

#include "Sail/entities/components/LightComponent.h"
#include "Sail/entities/ECS.h"
#include "Sail/graphics/light/LightSetup.h"

LightSystem::LightSystem() : BaseComponentSystem() {
	// TODO: System owner should check if this is correct
	registerComponent<LightComponent>(true, true, true);
}

LightSystem::~LightSystem() {
}

//check and update all lights for all entities
void LightSystem::updateLights(LightSetup* lightSetup) {
	for (auto e : entities) {
		LightComponent* lc = e->getComponent<LightComponent>();
		lightSetup->addPointLight(lc->getPointLight());
	}
}