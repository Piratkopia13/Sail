#include "pch.h"

#include "AudioComponent.h"

AudioComponent::AudioComponent() {
	for (int i = 0; i < SoundType::COUNT; i++) {

		m_soundEffects[i] = "";
		m_soundEffectTimers[i] = 0.0f;
		m_isPlaying[i] = false;
	}
}

AudioComponent::~AudioComponent() {

}
