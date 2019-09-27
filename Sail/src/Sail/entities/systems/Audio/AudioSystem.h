#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include "..//BaseComponentSystem.h"
#include "..//Sail/src/API/Audio/AudioEngine.h"

class AudioComponent;

class AudioSystem final : public BaseComponentSystem {
public:
	AudioSystem();
	~AudioSystem();

	AudioEngine* getAudioEngine();

	void update(float dt) override;

	void stop() override;

private:
	AudioEngine m_audioEngine;
};

#endif
