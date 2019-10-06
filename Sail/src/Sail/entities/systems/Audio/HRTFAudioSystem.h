#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"

class SoundComponent;
class OmnidirectionalSound;
class Camera;

class HRTFAudioSystem final : public BaseComponentSystem {
public:
	HRTFAudioSystem();
	~HRTFAudioSystem();


	void update(float dt) override;
	void update(Camera& cam);

	void initializeSound(OmnidirectionalSound& sc);
	void updateSoundWithNewPosition(OmnidirectionalSound& sound, Camera& cam);
	// TODO:
	//void update(float3 cameraPos, float3 fameraRot, etc)


};

