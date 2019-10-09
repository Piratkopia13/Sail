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
	}

	//listener.pCone = nullptr;
}

AudioComponent::~AudioComponent() {
}

void AudioComponent::defineSound(
	SoundType::SoundType type,
	std::string filename,
	float soundLength,
	const glm::vec3& positionOffset,
	bool playOnce) {

	m_soundEffects[type] = filename;
	m_soundEffectLengths[type] = soundLength;
	m_playOnce[type] = playOnce;
	m_positionalOffset[type] = positionOffset;
}
