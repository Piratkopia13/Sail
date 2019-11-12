#include "pch.h"
#include "SanitySoundSystem.h"

#include "Sail/entities/components/TransformComponent.h"
#include "Sail/entities/components/SanityComponent.h"
#include "Sail/entities/components/AudioComponent.h"
#include "Sail/entities/components/LocalOwnerComponent.h"

#include "Sail/entities/Entity.h"

SanitySoundSystem::SanitySoundSystem() {
	registerComponent<SanityComponent>(true, true, false);
	registerComponent<TransformComponent>(true, true, false);
	registerComponent<AudioComponent>(true, true, true);
	registerComponent<LocalOwnerComponent>(true, false, false);
}

SanitySoundSystem::~SanitySoundSystem() {
}

void SanitySoundSystem::update(float dt) {

	for (auto e : entities) {
		SanityComponent* sc = e->getComponent<SanityComponent>();
		TransformComponent* tc = e->getComponent<TransformComponent>();
		AudioComponent* ac = e->getComponent<AudioComponent>();

		float volume = 1 - (sc->sanity / 100.0f); //gives value between 0-1. Volume = 1 when sanity = 0
		
		//TODO: Set Value
		//ac->setVolume["Insanity"] = volume
	}

}

bool SanitySoundSystem::onEvent(const Event& event) {
	return false;
}
