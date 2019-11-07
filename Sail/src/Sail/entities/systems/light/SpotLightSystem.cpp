#include "pch.h"
#include "SpotLightSystem.h"

#include "Sail/entities/components/SpotlightComponent.h"
#include "Sail/entities/components/TransformComponent.h"
#include "Sail/entities/components/MovementComponent.h"
#include "Sail/entities/ECS.h"
#include "Sail/graphics/light/LightSetup.h"

SpotLightSystem::SpotLightSystem() : BaseComponentSystem() {
	registerComponent<SpotlightComponent>(true, true, true);
	registerComponent<TransformComponent>(true, true, false);
}

SpotLightSystem::~SpotLightSystem() {

}

//check and update all lights for all entities
void SpotLightSystem::updateLights(LightSetup* lightSetup, float alpha) {
	int id = 0;
	lightSetup->getSLs().clear();
	for (auto e : entities) {
		SpotlightComponent* sc = e->getComponent<SpotlightComponent>();

		if (sc->isOn != sc->wasOn) {
			MovementComponent* mc = e->getComponent<MovementComponent>();

			if (mc) {
				if (sc->isOn) {
					mc->rotation.y += 4;
				} else {
					mc->rotation.y -= 4;
				}
			}

			sc->wasOn = sc->isOn;
		}


		if (!sc->isOn) {
			continue;
		}

		TransformComponent* t = e->getComponent<TransformComponent>();

		sc->light_entityRotated = sc->light;
		sc->light_entityRotated.setIndex(id++);
		sc->light_entityRotated.setDirection(glm::rotate(t->getInterpolatedRotation(alpha), sc->light.getDirection()));
		sc->light_entityRotated.setPosition(sc->light.getPosition() + t->getInterpolatedTranslation(alpha));
		
		lightSetup->addSpotLight(sc->light_entityRotated);
	}
}

void SpotLightSystem::toggleONOFF() {
	for (auto e : entities) {
		SpotlightComponent* sc = e->getComponent<SpotlightComponent>();
		sc->isOn = !sc->isOn;
	}
}
