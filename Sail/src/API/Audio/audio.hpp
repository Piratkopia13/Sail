#ifndef AUDIO_HPP
#define AUDIO_HPP

#include "Xaudio2.h"

#define SOUND_COUNT 10
#define STREAMING_BUFFER_SIZE 65536
#define MAX_BUFFER_COUNT 3

class Audio
{
public:
	Audio();
	~Audio();

	void loadSound(const std::string &filename);
	int playSound(const std::string& filename);
	void stopSpecificSound(int index);
	void stopAllSounds();

	void updateAudio();

private:
	bool m_singlePressBool1 = true;
	bool m_singlePressBool2 = true;

	// Main 'interface' object managing all audio engine states
	IXAudio2* m_xAudio2 = nullptr;
	// Represents the audio output device
	IXAudio2MasteringVoice* m_masterVoice = nullptr;
	// Represents one loaded sound in the form of an 'object'
	IXAudio2SourceVoice* m_sourceVoice[SOUND_COUNT];

	/*BYTE m_streamBuffers[MAX_BUFFER_COUNT][STREAMING_BUFFER_SIZE];*/
	/*OVERLAPPED m_overlapped = { 0 };*/
	int m_currIndex = 0;

	// PRIVATE FUNCTION
	//-----------------
	void initXAudio2();
	// ----------------
};

#endif
