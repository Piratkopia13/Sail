#include "pch.h"
#include "AudioData.h"

AudioData::AudioData() {

}

AudioData::AudioData(const std::string& filename, IXAudio2* xAudio2) {
	this->load(filename, xAudio2);
}

AudioData::~AudioData() {

}

void AudioData::load(const std::string& filename, IXAudio2* xAudio2) {
	Fileloader::WAVLoader WAVLoader(filename, xAudio2, m_data);
}

IXAudio2SourceVoice* AudioData::getSourceVoice() {
	return m_data.m_sourceVoice;
}

XAUDIO2_BUFFER* AudioData::getSoundBuffer() {
	return &m_data.m_soundBuffer;
}