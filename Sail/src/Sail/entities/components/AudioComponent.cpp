#include "pch.h"

#include "AudioComponent.h"

AudioComponent::AudioComponent() {
	for (int i = 0; i < SoundType::COUNT; i++) {

		m_soundEffects[i] = "";
		m_soundID[i] = -1;
		m_soundEffectTimers[i] = 0.0f;
		m_soundEffectLengths[i] = 0.0f;
		m_isPlaying[i] = false;
		m_playOnce[i] = true;
		m_isInitialized[i] = false;
		m_volume[i] = 1.0f;
	}
}

AudioComponent::~AudioComponent() {
}

void AudioComponent::defineSound(
	SoundType::SoundType type,
	const std::string& filename,
	float soundLength,
	bool playOnce,
	float volume,
	const glm::vec3& positionOffset) {

	m_soundEffects[type] = filename;
	m_soundEffectLengths[type] = soundLength;
	m_playOnce[type] = playOnce;
	m_positionalOffset[type] = positionOffset;
	m_volume[type] = volume;
}
