#include "pch.h"

#include "AudioComponent.h"

AudioComponent::AudioComponent() {
	for (int i = 0; i < SoundType::COUNT; i++) {

		m_soundEffects[i] = "";
		m_soundEffectTimers[i] = 0.0f;
		m_soundEffectThresholds[i] = 0.0f;
		m_isPlaying[i] = false;
		m_isLooping[i] = false;
	}
}

AudioComponent::~AudioComponent() {

}

void AudioComponent::defineSound(SoundType::SoundType type, std::string filename, float dtThreshold, bool isLooping) {

	m_soundEffects[type] = filename;
	m_soundEffectThresholds[type] = dtThreshold;
	m_isLooping[type] = isLooping;
}