#pragma once

#include "..//BaseComponentSystem.h"
#include "Sail/events/EventReceiver.h"
#include "AudioData.h"

class AudioComponent;
class AudioEngine;
class Camera;
class XAUDIO2FX_REVERB_PARAMETERS;

class AudioSystem final : public BaseComponentSystem, public EventReceiver {
public:
	AudioSystem();
	~AudioSystem();

	AudioEngine* getAudioEngine();

	void initialize();
	void update(Camera& cam, float dt, float alpha);
	void stop() override;

	bool hasUpdated = false;

	bool onEvent(const Event& event) override;

private:
	std::list<std::pair<std::string, Audio::StreamRequestInfo>>::iterator m_i;
	std::list<std::pair<std::string, Audio::StreamRequestInfo>>::iterator m_toBeDeleted;
	std::list<std::pair<std::string, std::pair<int, bool>>>::iterator m_j;
	std::list<std::pair<std::string, std::pair<int, bool>>>::iterator m_k;
	std::list<std::pair<std::string, std::pair<int, bool>>>::iterator m_streamToBeDeleted;

	std::string m_filename = "";
	float m_volume = 1.0f;
	bool m_isPositionalAudio;
	bool m_isLooping;
	int m_streamIndex = 0;

	bool m_hasOutputDevices = true;

	AudioEngine* m_audioEngine;
	int m_currStreamIndex = 0;

	void startPlayingRequestedStream(Entity* e, AudioComponent* audioC);
	void stopPlayingRequestedStream(Entity* e, AudioComponent* audioC);
	void updateStreamPosition(Entity* e, Camera& cam, float alpha);

	void updateProjectileLowPass(Audio::SoundInfo_General* general);

	void hotFixAmbiance(Entity* e, AudioComponent* audioC);
};

