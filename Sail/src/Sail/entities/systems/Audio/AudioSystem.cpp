#include "pch.h"

#include "AudioSystem.h"
#include "..//Sail/src/Sail/entities/components/AudioComponent.h"
#include "..//Sail/src/Sail/entities/components/PhysicsComponent.h"
#include "..//Sail/src/Sail/graphics/camera/PerspectiveCamera.h"
#include "..//..//Entity.h"
#include <iterator>
#include "..//Sail/src/Sail/Application.h"

AudioSystem::AudioSystem() {
}

AudioSystem::~AudioSystem() {
	m_audioEngine.stopAllSounds();
}

void AudioSystem::initialize(PerspectiveCamera* camPtr) {
	// TODO: System owner should check if this is correct
	registerComponent<AudioComponent>(true, true, true);
	m_audioEngine.loadSound("../Audio/footsteps_1.wav");
	m_audioEngine.loadSound("../Audio/jump.wav");

	m_camPtr = camPtr;
}

void AudioSystem::update(float dt) {

	AudioComponent* audioC = nullptr;
	PhysicsComponent* physicsC = nullptr;

	glm::vec3 dir;
	glm::vec3 up;
	glm::vec3 pos;
	glm::vec3 vel;

	for (auto& e : entities) {

		audioC = e->getComponent<AudioComponent>();
		physicsC = e->getComponent<PhysicsComponent>();

		dir = m_camPtr->getDirection();
		up = m_camPtr->getUp();
		pos = m_camPtr->getPosition();
		vel = physicsC->velocity;

		audioC->listener.OrientFront = X3DAUDIO_VECTOR(dir.x, dir.y, dir.z);
		audioC->listener.OrientTop = X3DAUDIO_VECTOR(up.x, up.y, up.z);
		audioC->listener.Position = X3DAUDIO_VECTOR(0,0,0);
		audioC->listener.Velocity = X3DAUDIO_VECTOR(vel.x, vel.y, vel.z);

		if (audioC != nullptr) {

			// Playing NORMAL sounds
			for (int i = 0; i < SoundType::COUNT; i++) {

				if (audioC->m_isPlaying[i]) {
					if (audioC->m_soundEffectTimers[i] == 0.0f) {
						audioC->m_soundID[i] = m_audioEngine.playSound(audioC->m_soundEffects[i], audioC->listener);
						audioC->m_soundEffectTimers[i] += dt;

						audioC->m_isPlaying[i] = !audioC->m_playOnce[i]; // NOTE: Opposite, hence '!'
					}

					else {
						audioC->m_soundEffectTimers[i] += dt;

						if (audioC->m_soundEffectTimers[i] > audioC->m_soundEffectThresholds[i]) {
							audioC->m_soundEffectTimers[i] = 0.0f;
						}
					}
				}

				else if (audioC->m_soundEffectTimers[i] != 0.0f && !audioC->m_playOnce[i]) {
					m_audioEngine.stopSpecificSound(audioC->m_soundID[i]);
					audioC->m_soundEffectTimers[i] = 0.0f;
				}

				// NOTE:
				//		This is a correct solution for FADING, I believe, however it has to be done using a thread
				//
				//else if (audioC->m_soundEffectTimers[i] != 0.0f && !audioC->m_playOnce[i]) {
					//float volumeHolder = m_audioEngine.getSoundVolume(audioC->m_soundID[i]);
					//if (volumeHolder > 0.0f) {
					//	m_audioEngine.setSoundVolume(audioC->m_soundID[i], (volumeHolder - (1.0f * dt)));
					//}
					//else {
					//	m_audioEngine.stopSpecificSound(audioC->m_soundID[i]);
					//	audioC->m_soundEffectTimers[i] = 0.0f;
					//}
				//}
			}
		}

			// Playing STREAMED sounds
		std::list<std::pair<std::string, bool>>::iterator i;
		std::list<std::pair<std::string, bool>>::iterator toBeDeleted;
		std::list<std::pair<std::string, int>>::iterator j;
		std::list<std::pair<std::string, int>>::iterator streamToBeDeleted;
		std::string filename = "";
		int streamIndex = 0;

		for (i = audioC->m_streamingRequests.begin(); i != audioC->m_streamingRequests.end();) {

			if (i->second == true) {

				filename = i->first;
				toBeDeleted = i;
				i++;

				streamIndex = m_audioEngine.getAvailableStreamIndex();

				if (streamIndex == -1) {
					Logger::Error("Too many sounds already streaming; failed to stream another one!");
				}
				else {

					Application::getInstance()->pushJobToThreadPool(
						[this, filename, streamIndex](int id) {
							return m_audioEngine.streamSound(filename, streamIndex);
						});

					audioC->m_currentlyStreaming.emplace_back(filename, streamIndex);
					audioC->m_streamingRequests.erase(toBeDeleted);
				}
			}
			else/*if (i.second == false)*/ {

				filename = i->first;
				toBeDeleted = i;
				i++;

				for (j = audioC->m_currentlyStreaming.begin(); j != audioC->m_currentlyStreaming.end();) {

					streamToBeDeleted = j;
					j++;

					if (streamToBeDeleted->first == filename) {

						bool expectedValue = false;
						while (!m_audioEngine.m_streamLocks[streamToBeDeleted->second].compare_exchange_strong(expectedValue, true));

						m_audioEngine.stopSpecificStream(streamToBeDeleted->second);
						audioC->m_currentlyStreaming.erase(streamToBeDeleted);

						break;
					}
				}
				audioC->m_streamingRequests.erase(toBeDeleted);
			}
		}
	}
}

void AudioSystem::stop() {
	m_audioEngine.stopAllStreams();
}

AudioEngine* AudioSystem::getAudioEngine() {
	return &m_audioEngine;
}