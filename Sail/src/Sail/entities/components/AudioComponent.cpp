#include "pch.h"

#include "AudioComponent.h"

AudioComponent::AudioComponent() {}

AudioComponent::~AudioComponent() {}

void AudioComponent::streamSoundRequest_HELPERFUNC(std::string filename, bool startTRUE_stopFALSE, float volume, bool isLooping) {

	m_streamingRequests.push_back(std::pair(filename, startTRUE_stopFALSE));
	m_Vol_isLooping_Requests.push_back(std::pair(filename, std::pair(volume, isLooping)));
}

void AudioComponent::defineSound(Audio::SoundType type, Audio::SoundInfo info) {
	m_sounds[type] = info;
}
