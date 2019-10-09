#include "pch.h"

#include "AudioSystem.h"

#include "..//Sail/src/API/Audio/AudioEngine.h"
#include "..//Sail/src/Sail/Application.h"
#include "..//Sail/src/Sail/entities/components/AudioComponent.h"
#include "..//Sail/src/Sail/entities/components/TransformComponent.h"
#include "..//Sail/src/Sail/graphics/camera/Camera.h"
#include "..//..//Entity.h"

#include <iterator>

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

// TODO: move to constructor?
void AudioSystem::initialize() {
	m_audioEngine->loadSound("../Audio/footsteps_1.wav");
	m_audioEngine->loadSound("../Audio/jump.wav");
	m_audioEngine->loadSound("../Audio/guitar.wav");
}

void AudioSystem::update(Camera& cam, float dt, float alpha) {
	// Loop through entities
	for (auto e : entities) {
		auto audioC = e->getComponent<AudioComponent>();
		// Loop through sounds
		for (int i = 0; i < SoundType::COUNT; i++) {
			if (audioC->m_isPlaying[i]) {
				// Initialize the sound if that hasn't been done already
				if (!audioC->m_isInitialized[i]) {
					audioC->m_soundID[i] = m_audioEngine->initializeSound(audioC->m_soundEffects[i], audioC->m_volume[i]);
					audioC->m_isInitialized[i] = true;
					audioC->m_soundEffectTimers[i] = 0.0f;
				}

				// Start playing the sound if it's not already playing
				if (audioC->m_soundEffectTimers[i] == 0.0f) {
					m_audioEngine->startSpecificSound(audioC->m_soundID[i]);
				}

				// Update the sound with the current positions if it's playing
				if (audioC->m_soundEffectTimers[i] <= audioC->m_soundEffectLengths[i]) {
					m_audioEngine->updateSoundWithCurrentPosition(
						audioC->m_soundID[i], cam, *e->getComponent<TransformComponent>(), 
						audioC->m_positionalOffset[i], alpha);					
					
					audioC->m_soundEffectTimers[i] += dt;
				} else {
					audioC->m_soundEffectTimers[i] = 0.0f; // Reset the sound effect to its beginning
					m_audioEngine->stopSpecificSound(audioC->m_soundID[i]);
					
					// If the sound isn't looping then make it stop
					if (audioC->m_playOnce[i] == true) {
						audioC->m_isPlaying[i] = false;
						audioC->m_isInitialized[i] = false;
					}
				}
			// If the sound should no longer be playing stop it and reset its timer
			} else if (audioC->m_soundEffectTimers[i] != 0.0f) {
				m_audioEngine->stopSpecificSound(audioC->m_soundID[i]);
				audioC->m_soundEffectTimers[i] = 0.0f;
			}
		}
	}


	// STREAMING: This part has not been tested in a while
	{
		//		// Playing STREAMED sounds
		//	std::list<std::pair<std::string, bool>>::iterator i;
		//	std::list<std::pair<std::string, bool>>::iterator toBeDeleted;
		//	std::list<std::pair<std::string, int>>::iterator j;
		//	std::list<std::pair<std::string, int>>::iterator streamToBeDeleted;
		//	std::string filename = "";
		//	int streamIndex = 0;

		//	for (i = audioC->m_streamingRequests.begin(); i != audioC->m_streamingRequests.end();) {

		//		if (i->second == true) {

		//			filename = i->first;
		//			toBeDeleted = i;
		//			i++;

		//			streamIndex = m_audioEngine.getAvailableStreamIndex();

		//			if (streamIndex == -1) {
		//				Logger::Error("Too many sounds already streaming; failed to stream another one!");
		//			}
		//			else {

		//				Application::getInstance()->pushJobToThreadPool(
		//					[this, filename, streamIndex](int id) {
		//						return m_audioEngine.streamSound(filename, streamIndex);
		//					});

		//				audioC->m_currentlyStreaming.emplace_back(filename, streamIndex);
		//				audioC->m_streamingRequests.erase(toBeDeleted);
		//			}
		//		}
		//		else/*if (i.second == false)*/ {

		//			filename = i->first;
		//			toBeDeleted = i;
		//			i++;

		//			for (j = audioC->m_currentlyStreaming.begin(); j != audioC->m_currentlyStreaming.end();) {

		//				streamToBeDeleted = j;
		//				j++;

		//				if (streamToBeDeleted->first == filename) {

		//					bool expectedValue = false;
		//					while (!m_audioEngine.m_streamLocks[streamToBeDeleted->second].compare_exchange_strong(expectedValue, true));

		//					m_audioEngine.stopSpecificStream(streamToBeDeleted->second);
		//					audioC->m_currentlyStreaming.erase(streamToBeDeleted);

		//					break;
		//				}
		//			}
		//			audioC->m_streamingRequests.erase(toBeDeleted);
		//		}
		//	}
		//}
	}
}


void AudioSystem::stop() {
	m_audioEngine->stopAllStreams();
}

AudioEngine* AudioSystem::getAudioEngine() {
	return m_audioEngine;
}