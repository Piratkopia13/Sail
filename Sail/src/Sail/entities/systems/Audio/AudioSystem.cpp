#include "pch.h"

#include "AudioSystem.h"
#include "..//Sail/src/Sail/entities/components/AudioComponent.h"
#include "..//..//Entity.h"
#include <iterator>

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
					}

					else {
						audioC->m_soundEffectTimers[i] += dt;

						if (audioC->m_soundEffectTimers[i] > audioC->m_soundEffectThresholds[i]) {
							audioC->m_soundEffectTimers[i] = 0.0f;
						}
					}
				}

				else if (audioC->m_soundEffectTimers[i] != 0.0f && !audioC->m_playOnce[i]) {
					m_audioEngine.stopSpecificSound(audioC->m_soundID[i]);
					audioC->m_soundEffectTimers[i] = 0.0f;
				}

				// NOTE:
				//		This is a correct solution for FADING, I believe, however it has to be done using a thread
				//
				//else if (audioC->m_soundEffectTimers[i] != 0.0f && !audioC->m_playOnce[i]) {
					//float volumeHolder = m_audioEngine.getSoundVolume(audioC->m_soundID[i]);
					//if (volumeHolder > 0.0f) {
					//	m_audioEngine.setSoundVolume(audioC->m_soundID[i], (volumeHolder - (1.0f * dt)));
					//}
					//else {
					//	m_audioEngine.stopSpecificSound(audioC->m_soundID[i]);
					//	audioC->m_soundEffectTimers[i] = 0.0f;
					//}
				//}
			}
		}

			// Playing STREAMED sounds
		std::unordered_map<std::string, bool>::iterator i = audioC->m_streamedSounds.begin();
		std::string filename = "";

			while (i != audioC->m_streamedSounds.end()) {
				if (i->second == true) {
					filename = i->first;
					i++;

					audioC->m_streamedSoundsID.insert({ filename, m_audioEngine.streamSound(filename) });
					audioC->m_streamedSounds.erase(filename);
					// NOTE: We are NOT incrementing because we JUST ERASED an element from the map
				}
				else/*if (i.second == false)*/ {
					filename = i->first;
					i++;

					m_audioEngine.stopSpecificStream(audioC->m_streamedSoundsID.find(filename)->second);
					audioC->m_streamedSounds.erase(filename);
					audioC->m_streamedSoundsID.erase(filename);
				}
			}
	}
}

void AudioSystem::stop() {
	m_audioEngine.stopAllStreams();
}

AudioEngine* AudioSystem::getAudioEngine() {
	return &m_audioEngine;
}