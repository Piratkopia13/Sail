#include "pch.h"

#include "AudioSystem.h"

#include "..//Sail/src/API/Audio/AudioEngine.h"
#include "../Sail/src/Sail/entities/systems/Audio/AudioData.h"
#include "..//Sail/src/Sail/Application.h"
#include "..//Sail/src/Sail/entities/components/AudioComponent.h"
#include "..//Sail/src/Sail/entities/components/TransformComponent.h"
#include "..//Sail/src/Sail/graphics/camera/Camera.h"
#include "..//..//Entity.h"

#include <iterator>

#include <mmeapi.h>		// Used to determine if there are outputDevices for sound
#include <windows.h>	//
#include <mmsystem.h>	//

AudioSystem::AudioSystem() : BaseComponentSystem() {
	registerComponent<AudioComponent>(true, true, true);
	registerComponent<TransformComponent>(true, true, false);

	m_audioEngine = SAIL_NEW AudioEngine();
	initialize();
}

AudioSystem::~AudioSystem() {
	m_audioEngine->stopAllSounds();
	delete m_audioEngine;
}

// TO DO: move to constructor?
void AudioSystem::initialize() {
#pragma region FOOTSTEPS
	m_audioEngine->loadSound("footsteps/footsteps_metal_1.wav");
	m_audioEngine->loadSound("footsteps/footsteps_metal_2.wav");
	m_audioEngine->loadSound("footsteps/footsteps_metal_3.wav");
	m_audioEngine->loadSound("footsteps/footsteps_metal_4.wav");
	m_audioEngine->loadSound("footsteps/footsteps_tile_1.wav");
	m_audioEngine->loadSound("footsteps/footsteps_tile_2.wav");
	m_audioEngine->loadSound("footsteps/footsteps_tile_3.wav");
	m_audioEngine->loadSound("footsteps/footsteps_tile_4.wav");
	m_audioEngine->loadSound("footsteps/footsteps_water_metal_1.wav");
	m_audioEngine->loadSound("footsteps/footsteps_water_metal_2.wav");
	m_audioEngine->loadSound("footsteps/footsteps_water_metal_3.wav");
	m_audioEngine->loadSound("footsteps/footsteps_water_metal_4.wav");
	m_audioEngine->loadSound("footsteps/footsteps_water_tile_1.wav");
	m_audioEngine->loadSound("footsteps/footsteps_water_tile_2.wav");
	m_audioEngine->loadSound("footsteps/footsteps_water_tile_3.wav");
	m_audioEngine->loadSound("footsteps/footsteps_water_tile_4.wav");
#pragma endregion
#pragma region JUMPING
	m_audioEngine->loadSound("jumping/jump.wav");
	m_audioEngine->loadSound("jumping/landing_ground.wav");
#pragma endregion
#pragma region WATERGUN
	m_audioEngine->loadSound("watergun/watergun_start.wav");
	m_audioEngine->loadSound("watergun/watergun_loop.wav");
	m_audioEngine->loadSound("watergun/watergun_end.wav");
	m_audioEngine->loadSound("watergun/watergun_reload.wav");
#pragma endregion
#pragma region IMPACTS
	m_audioEngine->loadSound("impacts/water_impact_enemy_candle.wav");
	m_audioEngine->loadSound("impacts/water_impact_my_candle.wav");
	m_audioEngine->loadSound("impacts/water_drip_1.wav");
	m_audioEngine->loadSound("impacts/water_drip_2.wav");
	m_audioEngine->loadSound("impacts/water_drip_3.wav");
	m_audioEngine->loadSound("impacts/water_drip_4.wav");
	m_audioEngine->loadSound("impacts/water_drip_5.wav");
	m_audioEngine->loadSound("impacts/water_drip_6.wav");
	m_audioEngine->loadSound("impacts/water_drip_7.wav");
#pragma endregion
#pragma region DEATHS
	m_audioEngine->loadSound("death/killing_blow.wav");
	m_audioEngine->loadSound("death/death_1.wav");
	m_audioEngine->loadSound("death/death_2.wav");
	m_audioEngine->loadSound("death/death_3.wav");
	m_audioEngine->loadSound("death/death_4.wav");
	m_audioEngine->loadSound("death/death_5.wav");
	m_audioEngine->loadSound("death/death_6.wav");
#pragma endregion
#pragma region MISCELLANEOUS
	m_audioEngine->loadSound("miscellaneous/re_ignition_candle.wav");
	m_audioEngine->loadSound("miscellaneous/guitar.wav");
#pragma endregion
}

void AudioSystem::update(Camera& cam, float dt, float alpha) {
	if (waveOutGetNumDevs() == 0) {
		m_hasOutputDevices = false;
	}
	if (!m_hasOutputDevices) {	// Only run audiosystem if there are outputdevices on the computer
		return;
	}
	for (auto e : entities) {
		auto audioC = e->getComponent<AudioComponent>();

		// - - - S O U N D S ---------------------------------------------------------------------------
		{
			int randomSoundIndex = 0;
			int soundPoolSize = 0;
			Audio::SoundInfo_General* soundGeneral;
			Audio::SoundInfo_Unique* soundUnique;

			for (int soundTypeIndex = 0; soundTypeIndex < Audio::SoundType::COUNT; soundTypeIndex++) {
				soundGeneral = &audioC->m_sounds[soundTypeIndex];

				soundPoolSize = audioData.m_soundsUnique[soundTypeIndex].size();
				if (soundPoolSize > 0) {

					if (soundGeneral->isPlaying) {

						// Starts a new sound from relevant pool of sounds IF NOT ALREADY PLAYING
						if (!soundGeneral->hasStartedPlaying) {

							if (soundPoolSize > 1) {
								randomSoundIndex = rand() % soundPoolSize;
								if (randomSoundIndex == soundGeneral->prevRandomNum) {
									randomSoundIndex++;
									randomSoundIndex = (randomSoundIndex % soundPoolSize);
								}
								soundGeneral->prevRandomNum = randomSoundIndex;
							}
							else {
								randomSoundIndex = 0;
							}

							// To make the code easier to read
							soundUnique = &audioData.m_soundsUnique[soundTypeIndex].at(randomSoundIndex);

							soundGeneral->soundID = m_audioEngine->beginSound(soundUnique->fileName, soundUnique->volume);
							soundGeneral->hasStartedPlaying = true;
							soundGeneral->durationElapsed = 0.0f;
							soundGeneral->currentSoundsLength = soundUnique->soundEffectLength;
							m_audioEngine->startSpecificSound(soundGeneral->soundID, soundUnique->volume);
						}

						// Update the sound with the current positions if it's playing
						if (soundGeneral->durationElapsed < soundGeneral->currentSoundsLength) {
							m_audioEngine->updateSoundWithCurrentPosition(
								soundGeneral->soundID, cam, *e->getComponent<TransformComponent>(),
								soundGeneral->positionalOffset, alpha);

							soundGeneral->durationElapsed += dt;
						}
						else {
							soundGeneral->durationElapsed = 0.0f; // Reset the sound effect to its beginning
							m_audioEngine->stopSpecificSound(soundGeneral->soundID);
							soundGeneral->hasStartedPlaying = false;

							soundGeneral->isPlaying = !soundGeneral->playOnce;
						}
						// If the sound should no longer be playing stop it and reset its timer
					}
					else if (soundGeneral->hasStartedPlaying) {
						m_audioEngine->stopSpecificSound(soundGeneral->soundID);
						soundGeneral->hasStartedPlaying = false;
						soundGeneral->durationElapsed = 0.0f;
					}
				}
			}
		}

		// - - - S T R E A M I N G  --------------------------------------------------------------------
		{
			m_filename = "";
			m_volume = 1.0f;
			m_streamIndex = 0;

			// Deal with requests
			for (m_i = audioC->m_streamingRequests.begin(); m_i != audioC->m_streamingRequests.end();) {

				// If the request wants to start
				if (m_i->second.startTRUE_stopFALSE == true) {
					// Start playing stream
					startPlayingRequestedStream(e, audioC);
				}
				// If the request wants to stop
				else {
					// Stop playing stream
					stopPlayingRequestedStream(e, audioC);
				}
			}

			// Hot fix for lab ambiance not playing after pause.
			//hotFixAmbiance(e, audioC);	// Commented-out because it causes a crash during death of other player

			// Per currently streaming sound
			for (m_k = audioC->m_currentlyStreaming.begin(); m_k != audioC->m_currentlyStreaming.end();) {
				// Update its position in the world
				updateStreamPosition(e, cam, alpha);
			}
		}
	}
}

void AudioSystem::stop() {
	m_audioEngine->stopAllStreams();
}

void AudioSystem::startPlayingRequestedStream(Entity* e, AudioComponent* audioC) {
	// Fetch found stream-request's filename
	m_filename = m_i->first;
	m_volume = m_i->second.volume;
	m_isPositionalAudio = m_i->second.isPositionalAudio;
	m_isLooping = m_i->second.isLooping;

	m_toBeDeleted = m_i;
	m_i++;

	m_streamIndex = m_audioEngine->getAvailableStreamIndex();

	std::string filename = m_filename;
	float volume = m_volume;
	bool isPositionalAudio = m_isPositionalAudio;
	bool isLooping = m_isLooping;
	int streamIndex = m_streamIndex;

	if (m_streamIndex == -1) {
		Logger::Error("Too many sounds already streaming; failed to stream another one!");
	}
	else {


		Application::getInstance()->pushJobToThreadPool(
			[this, filename, streamIndex, volume, isPositionalAudio, isLooping, audioC](int id) {
			return m_audioEngine->streamSound(m_filename, m_streamIndex, m_volume, m_isPositionalAudio, m_isLooping, audioC);
		});

		audioC->m_currentlyStreaming.emplace_back(m_filename, std::pair(m_streamIndex, m_isPositionalAudio));
		audioC->m_streamingRequests.erase(m_toBeDeleted);
	}
}

void AudioSystem::stopPlayingRequestedStream(Entity* e, AudioComponent* audioC) {
	m_filename = m_i->first;
	m_toBeDeleted = m_i;
	m_i++;

	for (m_j = audioC->m_currentlyStreaming.begin(); m_j != audioC->m_currentlyStreaming.end();) {

		m_streamToBeDeleted = m_j;
		m_j++;

		if (m_streamToBeDeleted->first == m_filename) {

			bool expectedValue = false;
			while (!m_audioEngine->m_streamLocks[m_streamToBeDeleted->second.first].compare_exchange_strong(expectedValue, true));

			m_audioEngine->stopSpecificStream(m_streamToBeDeleted->second.first);
			audioC->m_currentlyStreaming.erase(m_streamToBeDeleted);

			break;
		}
	}
	audioC->m_streamingRequests.erase(m_toBeDeleted);
}

void AudioSystem::updateStreamPosition(Entity* e, Camera& cam, float alpha) {
	if (m_k->second.second) {
		m_audioEngine->updateStreamWithCurrentPosition(
			m_k->second.first, cam, *e->getComponent<TransformComponent>(),
			glm::vec3{ 0.0f, 0.0f, 0.0f }, alpha);
	}

	m_k++;
}

void AudioSystem::hotFixAmbiance(Entity* e, AudioComponent* audioC) {
	if (e->getName() == "LabAmbiance") {
		// If it's not playing anything...
		if (audioC->m_currentlyStreaming.size() == 0) {
			// ... start playing ambiance again.
			audioC->streamSoundRequest_HELPERFUNC("res/sounds/ambient/ambiance_lab.xwb", true, 1.0f, false, true);
		}
	}
}

AudioEngine* AudioSystem::getAudioEngine() {
	return m_audioEngine;
}