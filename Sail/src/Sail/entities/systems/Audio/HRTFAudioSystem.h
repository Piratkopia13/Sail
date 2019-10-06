#pragma once

#include "Sail/entities/systems/BaseComponentSystem.h"

class SoundComponent;
class OmnidirectionalSound;

class HRTFAudioSystem final : public BaseComponentSystem {
public:
	HRTFAudioSystem();
	~HRTFAudioSystem();


	void update(float dt) override;

	void initializeSound(OmnidirectionalSound& sc);
	void updateSoundWithNewPosition(OmnidirectionalSound& sound);
	// TODO:
	//void update(float3 cameraPos, float3 fameraRot, etc)


};

