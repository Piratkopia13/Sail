#include "pch.h"

#include "AudioComponent.h"

AudioComponent::AudioComponent() {
	for (int i = 0; i < SoundType::COUNT; i++) {

		m_soundEffects[i] = "";
		m_soundID[i] = -1;
		m_soundEffectTimers[i] = 0.0f;
		m_soundEffectThresholds[i] = 0.0f;
		m_isPlaying[i] = false;
		m_playOnce[i] = true;
	}
}

AudioComponent::~AudioComponent() {
}

void AudioComponent::defineSound(SoundType::SoundType type, std::string filename, float dtThreshold, bool playOnce) {

	m_soundEffects[type] = filename;
	m_soundEffectThresholds[type] = dtThreshold;
	m_playOnce[type] = playOnce;
}

void AudioComponent::defineStreamedSound(std::string filename) {
	m_streamedSounds.insert({ filename, false });
}
