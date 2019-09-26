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
	m_audioEngine.stopAllSounds();
}

void AudioSystem::update(float dt) {

	AudioComponent* audioC = nullptr;

	for (auto& e : entities) {

		audioC = e->getComponent<AudioComponent>();

		if (audioC != nullptr) {

			// Playing NORMAL sounds
			for (int i = 0; i < SoundType::COUNT; i++) {

				if (audioC->m_isPlaying[i]) {
					if (audioC->m_soundEffectTimers[i] == 0.0f) {
						audioC->m_soundID[i] = m_audioEngine.playSound(audioC->m_soundEffects[i]);
						audioC->m_soundEffectTimers[i] += dt;

						audioC->m_isPlaying[i] = !audioC->m_playOnce[i]; // NOTE: Opposite, hence '!'
						std::cout << "LOOP-TIME!\n";
					}

					else {
						audioC->m_soundEffectTimers[i] += dt;
						std::cout << audioC->m_soundEffectTimers[SoundType::RUN] << "\n";

						if (audioC->m_soundEffectTimers[i] > audioC->m_soundEffectThresholds[i]) {
							audioC->m_soundEffectTimers[i] = 0.0f;
						}
					}
				}

				else if (audioC->m_soundEffectTimers[i] != 0.0f && !audioC->m_playOnce[i]) {
					m_audioEngine.stopSpecificSound(audioC->m_soundID[i]);
					audioC->m_soundEffectTimers[i] = 0.0f;
				}
			}

			// Playing STREAMED sounds
			for (auto&& i : audioC->m_streamedSounds) {
				if (i.second == true) {

				}
			}
		}
	}
}