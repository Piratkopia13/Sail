#ifndef AUDIO_DATA_H
#define AUDIO_DATA_H

#include <string>
#include "ResourceFormat.h"
#include "loaders/WAVLoader.h"

class AudioData {

public:
	AudioData();
	AudioData(const std::string& filename, IXAudio2* xAudio2);
	~AudioData();

	void load(const std::string& filename, IXAudio2* xAudio2);

	IXAudio2SourceVoice* getSourceVoice();
	XAUDIO2_BUFFER* getSoundBuffer();

private:
	ResourceFormat::AudioData m_data;
};

#endif
