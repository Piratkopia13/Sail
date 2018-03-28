#pragma once

#include "Sound.h"

class AmbientSound : public Sound {

public:
	enum SoundState {
		Playing,
		Paused,
		Stopped
	};

public:
	AmbientSound();
	~AmbientSound();

	virtual HRESULT Initialize(IXAudio2* audioEngine, wchar_t* file);

	void Play(const bool looped = false, float volume = 1.0f);
	void Pause();
	void Stop();

	void setVolume(float volume = 1.0f);
	float getVolume();

private:
	IXAudio2SourceVoice* m_sourceVoice;

	bool m_playing;

	SoundState m_state;

};