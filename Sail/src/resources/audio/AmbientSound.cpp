#include "AmbientSound.h"

AmbientSound::AmbientSound() {}
AmbientSound::~AmbientSound() {}

HRESULT AmbientSound::Initialize(IXAudio2* audioEngine, wchar_t* file) {
	Sound::Initialize(audioEngine, file);

	WAVEFORMATEXTENSIBLE wfx = Sound::getWFX();
	audioEngine->CreateSourceVoice(&m_sourceVoice, (WAVEFORMATEX*)&wfx);

	m_playing = false;
	m_state = SoundState::Stopped;

	return S_OK;
}

void AmbientSound::Play(const bool looped, float volume) {
	switch (m_state) {

	case SoundState::Paused:
		m_sourceVoice->Start();
		m_state = SoundState::Playing;
		break;

	case SoundState::Stopped:
		XAUDIO2_BUFFER buffer = Sound::getBuffer();
		if (looped)
			buffer.LoopCount = XAUDIO2_LOOP_INFINITE;
		m_sourceVoice->SetVolume(volume);
		m_sourceVoice->SubmitSourceBuffer(&buffer);
		m_sourceVoice->Start();
		m_state = SoundState::Playing;
		break;

	default:
		break;

	}

}

void AmbientSound::Pause() {
	if(m_state == SoundState::Playing)
		m_sourceVoice->Stop();

	m_state = SoundState::Paused;
}

void AmbientSound::Stop() {
	if (m_state != SoundState::Stopped) {
		m_sourceVoice->Stop();
		m_sourceVoice->FlushSourceBuffers();
	}

	m_state = SoundState::Stopped;
}

void AmbientSound::setVolume(float volume) {
	m_sourceVoice->SetVolume(volume);
}

float AmbientSound::getVolume() {
	float* vol = nullptr;
	m_sourceVoice->GetVolume(vol);
	
	float _vol = 1.f;
	
	if (vol) {
		_vol = *vol;
		delete vol;
	}

	return _vol;
}