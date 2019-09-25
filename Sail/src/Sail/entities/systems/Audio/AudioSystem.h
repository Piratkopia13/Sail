#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include "..//BaseComponentSystem.h"
#include "..//Sail/src/API/Audio/AudioEngine.h"

class AudioSystem : public BaseComponentSystem {
public:
	AudioSystem();
	~AudioSystem();

	void update(float dt) override;

private:
	AudioEngine m_audioEngine;
};

#endif
