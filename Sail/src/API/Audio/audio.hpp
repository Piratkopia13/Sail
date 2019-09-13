#ifndef AUDIO_HPP
#define AUDIO_HPP

#include "Xaudio2.h"

#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'

#define SOUND_COUNT 3
#define STREAMING_BUFFER_SIZE 65536
#define MAX_BUFFER_COUNT 3

class Audio
{
public:
	Audio();
	~Audio();

	void loadSound(std::string const &filename);
	void loadCompressedSound(std::string const& filename, int index);
	void playSound(int index);
	void stopSound(int index);

	void updateAudio();

private:
	bool m_singlePressBool1 = true;
	bool m_singlePressBool2 = true;

	// Main 'interface' object managing all audio engine states
	IXAudio2* m_xAudio2 = nullptr;
	// Represents the audio output device
	IXAudio2MasteringVoice* m_masterVoice = nullptr;
	// Represents one loaded sound in the form of an 'object'
	//IXAudio2SourceVoice* m_sourceVoice[SOUND_COUNT];
	//
	//XAUDIO2_BUFFER m_soundBuffers[SOUND_COUNT] = { 0 };

	/*BYTE m_streamBuffers[MAX_BUFFER_COUNT][STREAMING_BUFFER_SIZE];*/
	/*OVERLAPPED m_overlapped = { 0 };*/
	/*int m_currIndex = 0;*/

	// PRIVATE FUNCTION
	//-----------------
	void initXAudio2();
	// ----------------
};

#endif
