#include "SoundManager.h"

#include <xaudio2.h>

#include "Sound.h"
#include "AmbientSound.h"

//--------------------------------------------------------------------------------------
// Helper macros
//--------------------------------------------------------------------------------------
#ifndef SAFE_RELEASE
#define SAFE_RELEASE(p)      { if(p) { (p)->Release(); (p)=nullptr; } }
#endif

/////////////////////////////////
/////// PUBLIC FUNCTIONS ////////
/////////////////////////////////

SoundManager::SoundManager() {
	CoInitializeEx( NULL, COINIT_MULTITHREADED );

	m_playSound = true;
	m_masterVolume = 1.f;
	m_effectsVolume = 1.f;
	m_ambientVolume = 1.f;

	HRESULT hr;
	if (FAILED(hr = XAudio2Create(&m_audioEngine, 0, XAUDIO2_DEFAULT_PROCESSOR))) {
		Logger::Warning("Failed to initialize the audio engine.");
		m_playSound = false;
	}

	if (FAILED(hr = m_audioEngine->CreateMasteringVoice(&m_masterVoice))) {
		Logger::Warning("Failed to encapsulate the audio device.");
		m_playSound = false;
	}

	if (m_playSound) {

		m_audioEngine->StartEngine();

		m_masterVoice->SetVolume(m_masterVolume);

		m_sourceVoices.resize(NUMBER_OF_CHANNELS);
		for (int i = 0; i < NUMBER_OF_CHANNELS; i++)
			m_sourceVoices[i] = nullptr;
		m_currSVIndex = 0;

		m_sounds.resize(SoundEffect::NumOfSoundEffects);
		m_ambientSounds.resize(Ambient::NumOfAmbientSounds);

		loadSoundEffect(SoundEffect::Explosion, L"res/sounds/effect/explosion.wav");
		loadSoundEffect(SoundEffect::Explosion2, L"res/sounds/effect/explosion2.wav");
		loadSoundEffect(SoundEffect::Laser, L"res/sounds/effect/laser.wav");
		loadSoundEffect(SoundEffect::Shock, L"res/sounds/effect/shock.wav");
		loadSoundEffect(SoundEffect::Male_Death, L"res/sounds/effect/death/male_death.wav");
		loadSoundEffect(SoundEffect::Goblin_Death, L"res/sounds/effect/death/goblin_death.wav");
		loadSoundEffect(SoundEffect::Pickup, L"res/sounds/effect/pickup.wav");
		loadSoundEffect(SoundEffect::Switch, L"res/sounds/effect/switch.wav");
		loadSoundEffect(SoundEffect::Select, L"res/sounds/effect/select.wav");
		loadSoundEffect(SoundEffect::Hook_Shot, L"res/sounds/effect/hook_shot.wav");
		loadSoundEffect(SoundEffect::Hook_Hit, L"res/sounds/effect/hook_hit.wav");
		loadSoundEffect(SoundEffect::Captured, L"res/sounds/effect/captured2.wav");
		loadSoundEffect(SoundEffect::Game_Over, L"res/sounds/effect/game_over.wav");
		loadSoundEffect(SoundEffect::Implosion, L"res/sounds/effect/implosion.wav");
		loadSoundEffect(SoundEffect::Respawn, L"res/sounds/effect/respawn.wav");

		loadAmbientSound(Ambient::Loop1, L"res/sounds/ambient/loop1.wav");
		loadAmbientSound(Ambient::Loop2, L"res/sounds/ambient/loop2.wav");
		loadAmbientSound(Ambient::Windows95, L"res/sounds/ambient/windows95.wav");
		loadAmbientSound(Ambient::Theme, L"res/sounds/ambient/theme.wav");
		loadAmbientSound(Ambient::Ambient_Capture, L"res/sounds/ambient/ambient_capture.wav");
		loadAmbientSound(Ambient::Battle_Sound, L"res/sounds/ambient/battle_sound.wav");
		loadAmbientSound(Ambient::Scoreboard, L"res/sounds/ambient/scoreboard.wav");
	}
}

SoundManager::~SoundManager() {

	m_audioEngine->StopEngine();
	SAFE_RELEASE(m_audioEngine);

	CoUninitialize();
}

void SoundManager::update(const float dt) {
	if (!m_playSound)
		return;

}

void SoundManager::playSoundEffect(const SoundEffect soundID, float volume, float pitch) {
	if (!m_playSound)
		return;

	if (soundID < 0 || soundID >= SoundEffect::NumOfSoundEffects) {
		Logger::Warning("Failed to play soundeffect since sound id was out of bounds. ID tried: " + soundID);
		return;
	}

	if (m_sourceVoices[m_currSVIndex])
		m_sourceVoices[m_currSVIndex]->DestroyVoice();

	float pit = min(MAX_PITCH, max(MIN_PITCH, pitch));
	float vol = min(XAUDIO2_MAX_VOLUME_LEVEL, max(-XAUDIO2_MAX_VOLUME_LEVEL, volume * m_effectsVolume));

	WAVEFORMATEXTENSIBLE wfx = m_sounds[soundID]->getWFX();
	XAUDIO2_BUFFER buffer = m_sounds[soundID]->getBuffer();
	m_audioEngine->CreateSourceVoice(&m_sourceVoices[m_currSVIndex], (WAVEFORMATEX*)&wfx);

	m_sourceVoices[m_currSVIndex]->SubmitSourceBuffer(&buffer);

	m_sourceVoices[m_currSVIndex]->SetVolume(vol);
	m_sourceVoices[m_currSVIndex]->SetFrequencyRatio(pit);

	m_sourceVoices[m_currSVIndex]->Start(0);
	m_currSVIndex++;
	m_currSVIndex = m_currSVIndex % NUMBER_OF_CHANNELS;
}

void SoundManager::playSoundEffectWithRndPitch(const SoundEffect soundID, float min, float max, float volume) {
	float pitch = Utils::rnd() * (max - min) + min;
	playSoundEffect(soundID, volume, pitch);
}


void SoundManager::playAmbientSound(const Ambient soundID, const bool looping, float volume) {
	if (!m_playSound)
		return;

	if (soundID < 0 || soundID >= Ambient::NumOfAmbientSounds) {
		Logger::Warning("Failed to play ambient sound since sound id was out of bounds. ID tried: " + soundID);
		return;
	}

	float vol = min(XAUDIO2_MAX_VOLUME_LEVEL, max(-XAUDIO2_MAX_VOLUME_LEVEL, volume * m_ambientVolume));
	m_ambientSounds[soundID]->Play(looping, vol);
	
	
}

void SoundManager::pauseAmbientSound(const Ambient soundID) {
	if (!m_playSound)
		return;

	if (soundID < 0 || soundID >= Ambient::NumOfAmbientSounds) {
		Logger::Warning("Failed to pause ambient sound since sound id was out of bounds. ID tried: " + soundID);
		return;
	}

	m_ambientSounds[soundID]->Pause();

}

void SoundManager::resumeAmbientSound(const Ambient soundID) {
	if (!m_playSound)
		return;

	if (soundID < 0 || soundID >= Ambient::NumOfAmbientSounds) {
		Logger::Warning("Failed to resume ambient sound since sound id was out of bounds. ID tried: " + soundID);
		return;
	}

	m_ambientSounds[soundID]->Play();

}

void SoundManager::stopAmbientSound(const Ambient soundID) {
	if (!m_playSound)
		return;

	if (soundID < 0 || soundID >= Ambient::NumOfAmbientSounds) {
		Logger::Warning("Failed to resume ambient sound since sound id was out of bounds. ID tried: " + soundID);
		return;
	}

	m_ambientSounds[soundID]->Stop();

}

void SoundManager::suspendAllSound() {
	if (!m_playSound)
		return;
	/*m_audioEngine->Suspend();*/
}

void SoundManager::resumeAllSound() {
	if (!m_playSound)
		return;
	/*m_audioEngine->Resume();*/
}

bool SoundManager::loadSoundEffect(const SoundEffect soundID, wchar_t* file) {
	if (!m_playSound)
		return false;

	if (soundID < 0 || soundID >= SoundEffect::NumOfSoundEffects) {
		Logger::Warning("Failed to load sound effect since sound id was out of bounds. ID tried: " + soundID);
		return false;
	}

	m_sounds[soundID] = std::make_unique<Sound>();
	m_sounds[soundID]->Initialize(m_audioEngine, file);
	if (!m_sounds[soundID]) {
		Logger::Warning("Failed to load sound effect with id: " + soundID);
		return false;
	}

	return true;
}

bool SoundManager::loadAmbientSound(const Ambient soundID, wchar_t* file) {
	if (!m_playSound)
		return false;
	
	if (soundID < 0 || soundID >= Ambient::NumOfAmbientSounds) {
		Logger::Warning("Failed to load ambient sound since sound id was out of bounds. ID tried: " + soundID);
		return false;
	}

	m_ambientSounds[soundID] = std::make_unique<AmbientSound>();
	m_ambientSounds[soundID]->Initialize(m_audioEngine, file);

	return true;
}

void SoundManager::setMasterVolume(const float& volume) {
	if (!m_playSound)
		return;

	m_masterVoice->SetVolume(volume);
}

void SoundManager::setAmbientVolume(const float& volume) {
	if (!m_playSound)
		return;

	float oldAmbient = m_ambientVolume;
	m_ambientVolume = volume;

	for (unsigned int i = 0; i < m_ambientSounds.size(); i++) {
		float oldVol = m_ambientSounds[i]->getVolume();
		oldVol /= oldAmbient;
		oldVol = min(XAUDIO2_MAX_VOLUME_LEVEL, max(-XAUDIO2_MAX_VOLUME_LEVEL, oldVol * m_ambientVolume));
		m_ambientSounds[i]->setVolume(volume);
	}
}

void SoundManager::setEffectsVolume(const float& volume) {
	if (!m_playSound)
		return;

	m_effectsVolume = volume;
}

float SoundManager::getMasterVolume() {
	if (!m_playSound)
		return 0.f;

	float volume;
	m_masterVoice->GetVolume(&volume);

	return volume;
}


/////////////////////////////////
/////// PRIVATE FUNCTIONS ///////
/////////////////////////////////