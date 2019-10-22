#include "pch.h"

#include "AudioComponent.h"

AudioComponent::AudioComponent() {}

AudioComponent::~AudioComponent() {}

void AudioComponent::streamSoundRequest_HELPERFUNC(std::string filename, bool startTRUE_stopFALSE, float volume, bool isPositionalAudio, bool isLooping) {

	Audio::StreamRequestInfo info;
	info.startTRUE_stopFALSE = startTRUE_stopFALSE;
	info.volume = volume;
	info.isPositionalAudio = isPositionalAudio;
	info.isLooping = isLooping;

	m_streamingRequests.push_back(std::pair(filename, info));
}

void AudioComponent::defineSound(Audio::SoundType type, Audio::SoundInfo info) {
	m_sounds[type] = info;
}
