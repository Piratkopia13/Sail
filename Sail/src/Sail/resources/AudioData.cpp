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

XAUDIO2_BUFFER* AudioData::getSoundBuffer() {
	return &m_data.m_soundBuffer;
}

WAVEFORMATEXTENSIBLE* AudioData::getFormat() {
	return &this->m_data.m_formatWAV;
}