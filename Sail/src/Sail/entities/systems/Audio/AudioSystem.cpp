#include "pch.h"

#include "AudioSystem.h"
#include "..//Sail/src/Sail/entities/components/AudioComponent.h"
#include "..//..//Entity.h"

AudioSystem::AudioSystem() {
	requiredComponentTypes.push_back(AudioComponent::ID);
	m_audioEngine.loadSound("../Audio/footsteps_1.wav");
	m_audioEngine.loadSound("../Audio/jump.wav");
}

AudioSystem::~AudioSystem() {

}

void AudioSystem::update(float dt) {

	AudioComponent* audioC = nullptr;

	for (auto& e : entities) {

		audioC = e->getComponent<AudioComponent>();

		if (audioC != nullptr) {

			for (int i = 0; i < SoundType::COUNT; i++) {

				if (audioC->m_isPlaying[i]) {

					audioC->m_soundEffectTimers[i] += dt;
					if (audioC->m_soundEffectTimers[i] > audioC->m_soundEffectThresholds[i]) {

						audioC->m_soundEffectTimers[i] = 0.0f;
						m_audioEngine.playSound(audioC->m_soundEffects[i]);
						audioC->m_isPlaying[i] = audioC->m_isLooping[i];
					}
				}

				else if (!audioC->m_isPlaying[SoundType::RUN]) {
					m_audioEngine.stopAllSounds();
				}
			}
		}
	}
}