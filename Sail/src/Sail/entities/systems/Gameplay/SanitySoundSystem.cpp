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

		float volume = (1.0f - (sc->sanity / 100.0f)); //gives value between 0-1. Volume = 1 when sanity = 0
		
		if (!switch_ambiance) {
			if (volume > 0.0f) {
				switch_ambiance = true;
				ac->streamSoundRequest_HELPERFUNC("res/sounds/sanity/insanity_ambiance.xwb", true, 1.0f, false, true);
			}
		}
		else if (switch_ambiance) {
			if (volume == 0.0f) {
				switch_ambiance = false;
				ac->streamSoundRequest_HELPERFUNC("res/sounds/sanity/insanity_ambiance.xwb", false, 1.0f, false, true);
			}
			else {
				float tempX = (1.0f - volume);
				// Logarithmic increase of volume = linear to our ears
				//ac->streamVolumeUpdate_HELPERFUNC("res/sounds/sanity/insanity_ambiance.xwb", volume);
			}
		}

		if (!switch_begin) {
			// Start when insanity isn't full
			if (volume > 0.1f) {
				switch_begin = true;
				heartBeatTimer = 0.0f;
			}
		}
		else if (switch_begin) {
			// Turn off/reset when insanity is full again
			if (volume == 0.0f) {
				switch_begin = false;
				switch_secondBeat = false;
			}
			// Play first heart beat
			if (heartBeatTimer < 0.1f) {
				
				ac->m_sounds[Audio::HEART_BEAT_FIRST].isPlaying = true;
				switch_secondBeat = true;
			}
			// Play second heart beat
			else if (switch_secondBeat && (heartBeatTimer > (heartSecondBeatThresh - ((heartSecondBeatThresh * 0.25f) * (volume * 2.0f))))) {
				ac->m_sounds[Audio::HEART_BEAT_SECOND].isPlaying = true;
				switch_secondBeat = false;
			}
			// Reset heart-sounds
			heartBeatTimer += dt;
			if (heartBeatTimer > (heartBeatResetThresh - ((heartBeatResetThresh * 0.35f) * (volume * 2.0f)))) {
				heartBeatTimer = 0.0f;
			}
		}


		//TODO: Set Value
		//ac->setVolume["Insanity"] = volume
	}

}

bool SanitySoundSystem::onEvent(const Event& event) {
	return false;
}
