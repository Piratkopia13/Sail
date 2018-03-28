#pragma once

#include <XAudio2.h>
#include "../../Sail.h"

#ifdef _XBOX //Big-Endian
#define fourccRIFF 'RIFF'
#define fourccDATA 'data'
#define fourccFMT 'fmt '
#define fourccWAVE 'WAVE'
#define fourccXWMA 'XWMA'
#define fourccDPDS 'dpds'
#endif

#ifndef _XBOX //Little-Endian
#define fourccRIFF 'FFIR'
#define fourccDATA 'atad'
#define fourccFMT ' tmf'
#define fourccWAVE 'EVAW'
#define fourccXWMA 'AMWX'
#define fourccDPDS 'sdpd'
#endif

class Sound;
class AmbientSound;
class SoundManager {

public:
	enum SoundEffect {
		Explosion,
		Explosion2,
		Shock,
		Laser,
		Male_Death,
		Goblin_Death,
		Pickup, 
		Switch,
		Select,
		Hook_Shot,
		Hook_Hit,
		Captured,
		Game_Over,
		Implosion,
		Respawn,
		NumOfSoundEffects
	};

	enum Ambient {
		Loop1,
		Loop2,
		Windows95,
		Theme,
		Ambient_Capture,
		Battle_Sound,
		Scoreboard,
		NumOfAmbientSounds
	};

	const static int NUMBER_OF_CHANNELS = 1024;


public:
	SoundManager();
	~SoundManager();

	void update(const float dt);

	/*
		Plays a sound effect

		@param soundID - ID of the sound effect that should be played
		@param volume - Optional volume of the sound
		@param pitch - Optional pitch of the sound
	*/
	void playSoundEffect(const SoundEffect soundID, float volume = 1.f, float pitch = 1.f);
	void playSoundEffectWithRndPitch(const SoundEffect soundID, float low, float high, float volume = 1.f);

	void playAmbientSound(const Ambient soundID, const bool looping = false, float volume = 1.0f);
	void pauseAmbientSound(const Ambient soundID);
	void resumeAmbientSound(const Ambient soundID);
	void stopAmbientSound(const Ambient soundID);

	void suspendAllSound();
	void resumeAllSound();

	bool loadSoundEffect(const SoundEffect soundID, wchar_t* file);
	bool loadAmbientSound(const Ambient soundID, wchar_t* file);

	void setMasterVolume(const float& volume);
	void setAmbientVolume(const float& volume);
	void setEffectsVolume(const float& volume);
	float getMasterVolume();

private:
	IXAudio2* m_audioEngine;
	IXAudio2MasteringVoice* m_masterVoice;
	std::vector<IXAudio2SourceVoice*> m_sourceVoices;

	std::vector<std::unique_ptr<Sound>> m_sounds;
	std::vector<std::unique_ptr<AmbientSound>> m_ambientSounds;

	bool m_retryAudio;
	int	m_currSVIndex;

	const float MIN_PITCH = 0.0009765625f;
	const float MAX_PITCH = 1024.f;

	bool m_playSound;

	float m_masterVolume;
	float m_effectsVolume;
	float m_ambientVolume;

};