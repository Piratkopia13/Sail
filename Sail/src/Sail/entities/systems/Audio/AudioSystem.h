#ifndef AUDIO_SYSTEM_H
#define AUDIO_SYSTEM_H

#include "..//BaseComponentSystem.h"
#include "..//Sail/src/API/Audio/AudioEngine.h"

class AudioComponent;
//class PerspectiveCamera;
class Camera;

class AudioSystem final : public BaseComponentSystem {
public:
	AudioSystem();
	~AudioSystem();

	AudioEngine* getAudioEngine();

	//void initialize(PerspectiveCamera* camPtr);
	void initialize();
	void update(float dt) override;
	void update(Camera& cam, float dt, float alpha);
	void stop() override;



	bool hasUpdated = false;

private:
	AudioEngine m_audioEngine;
	//PerspectiveCamera* m_camPtr;
	int m_currStreamIndex = 0;
};

#endif
